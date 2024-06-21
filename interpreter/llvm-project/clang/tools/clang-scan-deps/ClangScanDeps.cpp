//===- ClangScanDeps.cpp - Implementation of clang-scan-deps --------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/DependencyScanning/DependencyScanningService.h"
#include "clang/Tooling/DependencyScanning/DependencyScanningTool.h"
#include "clang/Tooling/DependencyScanning/DependencyScanningWorker.h"
#include "clang/Tooling/JSONCompilationDatabase.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileUtilities.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/JSON.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/ThreadPool.h"
#include "llvm/Support/Threading.h"
#include <mutex>
#include <optional>
#include <thread>

using namespace clang;
using namespace tooling::dependencies;

namespace {

class SharedStream {
public:
  SharedStream(raw_ostream &OS) : OS(OS) {}
  void applyLocked(llvm::function_ref<void(raw_ostream &OS)> Fn) {
    std::unique_lock<std::mutex> LockGuard(Lock);
    Fn(OS);
    OS.flush();
  }

private:
  std::mutex Lock;
  raw_ostream &OS;
};

class ResourceDirectoryCache {
public:
  /// findResourceDir finds the resource directory relative to the clang
  /// compiler being used in Args, by running it with "-print-resource-dir"
  /// option and cache the results for reuse. \returns resource directory path
  /// associated with the given invocation command or empty string if the
  /// compiler path is NOT an absolute path.
  StringRef findResourceDir(const tooling::CommandLineArguments &Args,
                            bool ClangCLMode) {
    if (Args.size() < 1)
      return "";

    const std::string &ClangBinaryPath = Args[0];
    if (!llvm::sys::path::is_absolute(ClangBinaryPath))
      return "";

    const std::string &ClangBinaryName =
        std::string(llvm::sys::path::filename(ClangBinaryPath));

    std::unique_lock<std::mutex> LockGuard(CacheLock);
    const auto &CachedResourceDir = Cache.find(ClangBinaryPath);
    if (CachedResourceDir != Cache.end())
      return CachedResourceDir->second;

    std::vector<StringRef> PrintResourceDirArgs{ClangBinaryName};
    if (ClangCLMode)
      PrintResourceDirArgs.push_back("/clang:-print-resource-dir");
    else
      PrintResourceDirArgs.push_back("-print-resource-dir");

    llvm::SmallString<64> OutputFile, ErrorFile;
    llvm::sys::fs::createTemporaryFile("print-resource-dir-output",
                                       "" /*no-suffix*/, OutputFile);
    llvm::sys::fs::createTemporaryFile("print-resource-dir-error",
                                       "" /*no-suffix*/, ErrorFile);
    llvm::FileRemover OutputRemover(OutputFile.c_str());
    llvm::FileRemover ErrorRemover(ErrorFile.c_str());
    std::optional<StringRef> Redirects[] = {
        {""}, // Stdin
        OutputFile.str(),
        ErrorFile.str(),
    };
    if (const int RC = llvm::sys::ExecuteAndWait(
            ClangBinaryPath, PrintResourceDirArgs, {}, Redirects)) {
      auto ErrorBuf = llvm::MemoryBuffer::getFile(ErrorFile.c_str());
      llvm::errs() << ErrorBuf.get()->getBuffer();
      return "";
    }

    auto OutputBuf = llvm::MemoryBuffer::getFile(OutputFile.c_str());
    if (!OutputBuf)
      return "";
    StringRef Output = OutputBuf.get()->getBuffer().rtrim('\n');

    Cache[ClangBinaryPath] = Output.str();
    return Cache[ClangBinaryPath];
  }

private:
  std::map<std::string, std::string> Cache;
  std::mutex CacheLock;
};

llvm::cl::opt<bool> Help("h", llvm::cl::desc("Alias for -help"),
                         llvm::cl::Hidden);

llvm::cl::OptionCategory DependencyScannerCategory("Tool options");

static llvm::cl::opt<ScanningMode> ScanMode(
    "mode",
    llvm::cl::desc("The preprocessing mode used to compute the dependencies"),
    llvm::cl::values(
        clEnumValN(ScanningMode::DependencyDirectivesScan,
                   "preprocess-dependency-directives",
                   "The set of dependencies is computed by preprocessing with "
                   "special lexing after scanning the source files to get the "
                   "directives that might affect the dependencies"),
        clEnumValN(ScanningMode::CanonicalPreprocessing, "preprocess",
                   "The set of dependencies is computed by preprocessing the "
                   "source files")),
    llvm::cl::init(ScanningMode::DependencyDirectivesScan),
    llvm::cl::cat(DependencyScannerCategory));

static llvm::cl::opt<ScanningOutputFormat> Format(
    "format", llvm::cl::desc("The output format for the dependencies"),
    llvm::cl::values(
        clEnumValN(ScanningOutputFormat::Make, "make",
                   "Makefile compatible dep file"),
        clEnumValN(ScanningOutputFormat::P1689, "p1689",
                   "Generate standard c++ modules dependency P1689 format"),
        clEnumValN(ScanningOutputFormat::Full, "experimental-full",
                   "Full dependency graph suitable"
                   " for explicitly building modules. This format "
                   "is experimental and will change.")),
    llvm::cl::init(ScanningOutputFormat::Make),
    llvm::cl::cat(DependencyScannerCategory));

static llvm::cl::opt<std::string> ModuleFilesDir(
    "module-files-dir",
    llvm::cl::desc(
        "The build directory for modules. Defaults to the value of "
        "'-fmodules-cache-path=' from command lines for implicit modules."),
    llvm::cl::cat(DependencyScannerCategory));

static llvm::cl::opt<bool> OptimizeArgs(
    "optimize-args",
    llvm::cl::desc("Whether to optimize command-line arguments of modules."),
    llvm::cl::init(false), llvm::cl::cat(DependencyScannerCategory));

static llvm::cl::opt<bool> EagerLoadModules(
    "eager-load-pcm",
    llvm::cl::desc("Load PCM files eagerly (instead of lazily on import)."),
    llvm::cl::init(false), llvm::cl::cat(DependencyScannerCategory));

llvm::cl::opt<unsigned>
    NumThreads("j", llvm::cl::Optional,
               llvm::cl::desc("Number of worker threads to use (default: use "
                              "all concurrent threads)"),
               llvm::cl::init(0), llvm::cl::cat(DependencyScannerCategory));

llvm::cl::opt<std::string>
    CompilationDB("compilation-database",
                  llvm::cl::desc("Compilation database"), llvm::cl::Optional,
                  llvm::cl::cat(DependencyScannerCategory));

llvm::cl::opt<std::string> P1689TargettedCommand(
    llvm::cl::Positional, llvm::cl::ZeroOrMore,
    llvm::cl::desc("The command line flags for the target of which "
                   "the dependencies are to be computed."));

llvm::cl::opt<std::string> ModuleName(
    "module-name", llvm::cl::Optional,
    llvm::cl::desc("the module of which the dependencies are to be computed"),
    llvm::cl::cat(DependencyScannerCategory));

llvm::cl::list<std::string> ModuleDepTargets(
    "dependency-target",
    llvm::cl::desc("The names of dependency targets for the dependency file"),
    llvm::cl::cat(DependencyScannerCategory));

llvm::cl::opt<bool> DeprecatedDriverCommand(
    "deprecated-driver-command", llvm::cl::Optional,
    llvm::cl::desc("use a single driver command to build the tu (deprecated)"),
    llvm::cl::cat(DependencyScannerCategory));

enum ResourceDirRecipeKind {
  RDRK_ModifyCompilerPath,
  RDRK_InvokeCompiler,
};

static llvm::cl::opt<ResourceDirRecipeKind> ResourceDirRecipe(
    "resource-dir-recipe",
    llvm::cl::desc("How to produce missing '-resource-dir' argument"),
    llvm::cl::values(
        clEnumValN(RDRK_ModifyCompilerPath, "modify-compiler-path",
                   "Construct the resource directory from the compiler path in "
                   "the compilation database. This assumes it's part of the "
                   "same toolchain as this clang-scan-deps. (default)"),
        clEnumValN(RDRK_InvokeCompiler, "invoke-compiler",
                   "Invoke the compiler with '-print-resource-dir' and use the "
                   "reported path as the resource directory. (deprecated)")),
    llvm::cl::init(RDRK_ModifyCompilerPath),
    llvm::cl::cat(DependencyScannerCategory));

llvm::cl::opt<bool> Verbose("v", llvm::cl::Optional,
                            llvm::cl::desc("Use verbose output."),
                            llvm::cl::init(false),
                            llvm::cl::cat(DependencyScannerCategory));

} // end anonymous namespace

/// Takes the result of a dependency scan and prints error / dependency files
/// based on the result.
///
/// \returns True on error.
static bool
handleMakeDependencyToolResult(const std::string &Input,
                               llvm::Expected<std::string> &MaybeFile,
                               SharedStream &OS, SharedStream &Errs) {
  if (!MaybeFile) {
    llvm::handleAllErrors(
        MaybeFile.takeError(), [&Input, &Errs](llvm::StringError &Err) {
          Errs.applyLocked([&](raw_ostream &OS) {
            OS << "Error while scanning dependencies for " << Input << ":\n";
            OS << Err.getMessage();
          });
        });
    return true;
  }
  OS.applyLocked([&](raw_ostream &OS) { OS << *MaybeFile; });
  return false;
}

static llvm::json::Array toJSONSorted(const llvm::StringSet<> &Set) {
  std::vector<llvm::StringRef> Strings;
  for (auto &&I : Set)
    Strings.push_back(I.getKey());
  llvm::sort(Strings);
  return llvm::json::Array(Strings);
}

static llvm::json::Array toJSONSorted(std::vector<ModuleID> V) {
  llvm::sort(V, [](const ModuleID &A, const ModuleID &B) {
    return std::tie(A.ModuleName, A.ContextHash) <
           std::tie(B.ModuleName, B.ContextHash);
  });

  llvm::json::Array Ret;
  for (const ModuleID &MID : V)
    Ret.push_back(llvm::json::Object(
        {{"module-name", MID.ModuleName}, {"context-hash", MID.ContextHash}}));
  return Ret;
}

// Thread safe.
class FullDeps {
public:
  void mergeDeps(StringRef Input, FullDependenciesResult FDR,
                 size_t InputIndex) {
    FullDependencies &FD = FDR.FullDeps;

    InputDeps ID;
    ID.FileName = std::string(Input);
    ID.ContextHash = std::move(FD.ID.ContextHash);
    ID.FileDeps = std::move(FD.FileDeps);
    ID.ModuleDeps = std::move(FD.ClangModuleDeps);

    std::unique_lock<std::mutex> ul(Lock);
    for (const ModuleDeps &MD : FDR.DiscoveredModules) {
      auto I = Modules.find({MD.ID, 0});
      if (I != Modules.end()) {
        I->first.InputIndex = std::min(I->first.InputIndex, InputIndex);
        continue;
      }
      Modules.insert(I, {{MD.ID, InputIndex}, std::move(MD)});
    }

    ID.DriverCommandLine = std::move(FD.DriverCommandLine);
    ID.Commands = std::move(FD.Commands);
    Inputs.push_back(std::move(ID));
  }

  void printFullOutput(raw_ostream &OS) {
    // Sort the modules by name to get a deterministic order.
    std::vector<IndexedModuleID> ModuleIDs;
    for (auto &&M : Modules)
      ModuleIDs.push_back(M.first);
    llvm::sort(ModuleIDs,
               [](const IndexedModuleID &A, const IndexedModuleID &B) {
                 return std::tie(A.ID.ModuleName, A.InputIndex) <
                        std::tie(B.ID.ModuleName, B.InputIndex);
               });

    llvm::sort(Inputs, [](const InputDeps &A, const InputDeps &B) {
      return A.FileName < B.FileName;
    });

    using namespace llvm::json;

    Array OutModules;
    for (auto &&ModID : ModuleIDs) {
      auto &MD = Modules[ModID];
      Object O{
          {"name", MD.ID.ModuleName},
          {"context-hash", MD.ID.ContextHash},
          {"file-deps", toJSONSorted(MD.FileDeps)},
          {"clang-module-deps", toJSONSorted(MD.ClangModuleDeps)},
          {"clang-modulemap-file", MD.ClangModuleMapFile},
          {"command-line", MD.BuildArguments},
      };
      OutModules.push_back(std::move(O));
    }

    Array TUs;
    for (auto &&I : Inputs) {
      Array Commands;
      if (I.DriverCommandLine.empty()) {
        for (const auto &Cmd : I.Commands) {
          Object O{
              {"input-file", I.FileName},
              {"clang-context-hash", I.ContextHash},
              {"file-deps", I.FileDeps},
              {"clang-module-deps", toJSONSorted(I.ModuleDeps)},
              {"executable", Cmd.Executable},
              {"command-line", Cmd.Arguments},
          };
          Commands.push_back(std::move(O));
        }
      } else {
        Object O{
            {"input-file", I.FileName},
            {"clang-context-hash", I.ContextHash},
            {"file-deps", I.FileDeps},
            {"clang-module-deps", toJSONSorted(I.ModuleDeps)},
            {"executable", "clang"},
            {"command-line", I.DriverCommandLine},
        };
        Commands.push_back(std::move(O));
      }
      TUs.push_back(Object{
          {"commands", std::move(Commands)},
      });
    }

    Object Output{
        {"modules", std::move(OutModules)},
        {"translation-units", std::move(TUs)},
    };

    OS << llvm::formatv("{0:2}\n", Value(std::move(Output)));
  }

private:
  struct IndexedModuleID {
    ModuleID ID;
    mutable size_t InputIndex;

    bool operator==(const IndexedModuleID &Other) const {
      return ID.ModuleName == Other.ID.ModuleName &&
             ID.ContextHash == Other.ID.ContextHash;
    }
  };

  struct IndexedModuleIDHasher {
    std::size_t operator()(const IndexedModuleID &IMID) const {
      using llvm::hash_combine;

      return hash_combine(IMID.ID.ModuleName, IMID.ID.ContextHash);
    }
  };

  struct InputDeps {
    std::string FileName;
    std::string ContextHash;
    std::vector<std::string> FileDeps;
    std::vector<ModuleID> ModuleDeps;
    std::vector<std::string> DriverCommandLine;
    std::vector<Command> Commands;
  };

  std::mutex Lock;
  std::unordered_map<IndexedModuleID, ModuleDeps, IndexedModuleIDHasher>
      Modules;
  std::vector<InputDeps> Inputs;
};

static bool handleFullDependencyToolResult(
    const std::string &Input,
    llvm::Expected<FullDependenciesResult> &MaybeFullDeps, FullDeps &FD,
    size_t InputIndex, SharedStream &OS, SharedStream &Errs) {
  if (!MaybeFullDeps) {
    llvm::handleAllErrors(
        MaybeFullDeps.takeError(), [&Input, &Errs](llvm::StringError &Err) {
          Errs.applyLocked([&](raw_ostream &OS) {
            OS << "Error while scanning dependencies for " << Input << ":\n";
            OS << Err.getMessage();
          });
        });
    return true;
  }
  FD.mergeDeps(Input, std::move(*MaybeFullDeps), InputIndex);
  return false;
}

class P1689Deps {
public:
  void printDependencies(raw_ostream &OS) {
    addSourcePathsToRequires();
    // Sort the modules by name to get a deterministic order.
    llvm::sort(Rules, [](const P1689Rule &A, const P1689Rule &B) {
      return A.PrimaryOutput < B.PrimaryOutput;
    });

    using namespace llvm::json;
    Array OutputRules;
    for (const P1689Rule &R : Rules) {
      Object O{{"primary-output", R.PrimaryOutput}};

      if (R.Provides) {
        Array Provides;
        Object Provided{{"logical-name", R.Provides->ModuleName},
                        {"source-path", R.Provides->SourcePath},
                        {"is-interface", R.Provides->IsStdCXXModuleInterface}};
        Provides.push_back(std::move(Provided));
        O.insert({"provides", std::move(Provides)});
      }

      Array Requires;
      for (const P1689ModuleInfo &Info : R.Requires) {
        Object RequiredInfo{{"logical-name", Info.ModuleName}};
        if (!Info.SourcePath.empty())
          RequiredInfo.insert({"source-path", Info.SourcePath});
        Requires.push_back(std::move(RequiredInfo));
      }

      if (!Requires.empty())
        O.insert({"requires", std::move(Requires)});

      OutputRules.push_back(std::move(O));
    }

    Object Output{
        {"version", 1}, {"revision", 0}, {"rules", std::move(OutputRules)}};

    OS << llvm::formatv("{0:2}\n", Value(std::move(Output)));
  }

  void addRules(P1689Rule &Rule) {
    std::unique_lock<std::mutex> LockGuard(Lock);
    Rules.push_back(Rule);
  }

private:
  void addSourcePathsToRequires() {
    llvm::DenseMap<StringRef, StringRef> ModuleSourceMapper;
    for (const P1689Rule &R : Rules)
      if (R.Provides && !R.Provides->SourcePath.empty())
        ModuleSourceMapper[R.Provides->ModuleName] = R.Provides->SourcePath;

    for (P1689Rule &R : Rules) {
      for (P1689ModuleInfo &Info : R.Requires) {
        auto Iter = ModuleSourceMapper.find(Info.ModuleName);
        if (Iter != ModuleSourceMapper.end())
          Info.SourcePath = Iter->second;
      }
    }
  }

  std::mutex Lock;
  std::vector<P1689Rule> Rules;
};

static bool
handleP1689DependencyToolResult(const std::string &Input,
                                llvm::Expected<P1689Rule> &MaybeRule,
                                P1689Deps &PD, SharedStream &Errs) {
  if (!MaybeRule) {
    llvm::handleAllErrors(
        MaybeRule.takeError(), [&Input, &Errs](llvm::StringError &Err) {
          Errs.applyLocked([&](raw_ostream &OS) {
            OS << "Error while scanning dependencies for " << Input << ":\n";
            OS << Err.getMessage();
          });
        });
    return true;
  }
  PD.addRules(*MaybeRule);
  return false;
}

/// Construct a path for the explicitly built PCM.
static std::string constructPCMPath(ModuleID MID, StringRef OutputDir) {
  SmallString<256> ExplicitPCMPath(OutputDir);
  llvm::sys::path::append(ExplicitPCMPath, MID.ContextHash,
                          MID.ModuleName + "-" + MID.ContextHash + ".pcm");
  return std::string(ExplicitPCMPath);
}

static std::string lookupModuleOutput(const ModuleID &MID, ModuleOutputKind MOK,
                                      StringRef OutputDir) {
  std::string PCMPath = constructPCMPath(MID, OutputDir);
  switch (MOK) {
  case ModuleOutputKind::ModuleFile:
    return PCMPath;
  case ModuleOutputKind::DependencyFile:
    return PCMPath + ".d";
  case ModuleOutputKind::DependencyTargets:
    // Null-separate the list of targets.
    return join(ModuleDepTargets, StringRef("\0", 1));
  case ModuleOutputKind::DiagnosticSerializationFile:
    return PCMPath + ".diag";
  }
  llvm_unreachable("Fully covered switch above!");
}

static std::string getModuleCachePath(ArrayRef<std::string> Args) {
  for (StringRef Arg : llvm::reverse(Args)) {
    Arg.consume_front("/clang:");
    if (Arg.consume_front("-fmodules-cache-path="))
      return std::string(Arg);
  }
  SmallString<128> Path;
  driver::Driver::getDefaultModuleCachePath(Path);
  return std::string(Path);
}

// getCompilationDataBase - If -compilation-database is set, load the
// compilation database from the specified file. Otherwise if the we're
// generating P1689 format, trying to generate the compilation database
// form specified command line after the positional parameter "--".
static std::unique_ptr<tooling::CompilationDatabase>
getCompilationDataBase(int argc, const char **argv, std::string &ErrorMessage) {
  llvm::InitLLVM X(argc, argv);
  llvm::cl::HideUnrelatedOptions(DependencyScannerCategory);
  if (!llvm::cl::ParseCommandLineOptions(argc, argv))
    return nullptr;

  if (!CompilationDB.empty())
    return tooling::JSONCompilationDatabase::loadFromFile(
        CompilationDB, ErrorMessage,
        tooling::JSONCommandLineSyntax::AutoDetect);

  if (Format != ScanningOutputFormat::P1689) {
    llvm::errs() << "the --compilation-database option: must be specified at "
                    "least once!";
    return nullptr;
  }

  // Trying to get the input file, the output file and the command line options
  // from the positional parameter "--".
  const char **DoubleDash = std::find(argv, argv + argc, StringRef("--"));
  if (DoubleDash == argv + argc) {
    llvm::errs() << "The command line arguments is required after '--' in "
                    "P1689 per file mode.";
    return nullptr;
  }
  std::vector<const char *> CommandLine(DoubleDash + 1, argv + argc);

  llvm::IntrusiveRefCntPtr<DiagnosticsEngine> Diags =
      CompilerInstance::createDiagnostics(new DiagnosticOptions);
  driver::Driver TheDriver(CommandLine[0], llvm::sys::getDefaultTargetTriple(),
                           *Diags);
  std::unique_ptr<driver::Compilation> C(
      TheDriver.BuildCompilation(CommandLine));
  if (!C)
    return nullptr;

  auto Cmd = C->getJobs().begin();
  auto CI = std::make_unique<CompilerInvocation>();
  CompilerInvocation::CreateFromArgs(*CI, Cmd->getArguments(), *Diags,
                                     CommandLine[0]);
  if (!CI)
    return nullptr;

  FrontendOptions &FEOpts = CI->getFrontendOpts();
  if (FEOpts.Inputs.size() != 1) {
    llvm::errs() << "Only one input file is allowed in P1689 per file mode.";
    return nullptr;
  }

  // There might be multiple jobs for a compilation. Extract the specified
  // output filename from the last job.
  auto LastCmd = C->getJobs().end();
  LastCmd--;
  if (LastCmd->getOutputFilenames().size() != 1) {
    llvm::errs() << "The command line should provide exactly one output file "
                    "in P1689 per file mode.\n";
  }
  StringRef OutputFile = LastCmd->getOutputFilenames().front();

  class InplaceCompilationDatabase : public tooling::CompilationDatabase {
  public:
    InplaceCompilationDatabase(StringRef InputFile, StringRef OutputFile,
                               ArrayRef<const char *> CommandLine)
        : Command(".", InputFile, {}, OutputFile) {
      for (auto *C : CommandLine)
        Command.CommandLine.push_back(C);
    }

    std::vector<tooling::CompileCommand>
    getCompileCommands(StringRef FilePath) const override {
      if (FilePath != Command.Filename)
        return {};
      return {Command};
    }

    std::vector<std::string> getAllFiles() const override {
      return {Command.Filename};
    }

    std::vector<tooling::CompileCommand>
    getAllCompileCommands() const override {
      return {Command};
    }

  private:
    tooling::CompileCommand Command;
  };

  return std::make_unique<InplaceCompilationDatabase>(
      FEOpts.Inputs[0].getFile(), OutputFile, CommandLine);
}

int main(int argc, const char **argv) {
  std::string ErrorMessage;
  std::unique_ptr<tooling::CompilationDatabase> Compilations =
      getCompilationDataBase(argc, argv, ErrorMessage);
  if (!Compilations) {
    llvm::errs() << ErrorMessage << "\n";
    return 1;
  }

  llvm::cl::PrintOptionValues();

  // The command options are rewritten to run Clang in preprocessor only mode.
  auto AdjustingCompilations =
      std::make_unique<tooling::ArgumentsAdjustingCompilations>(
          std::move(Compilations));
  ResourceDirectoryCache ResourceDirCache;

  AdjustingCompilations->appendArgumentsAdjuster(
      [&ResourceDirCache](const tooling::CommandLineArguments &Args,
                          StringRef FileName) {
        std::string LastO;
        bool HasResourceDir = false;
        bool ClangCLMode = false;
        auto FlagsEnd = llvm::find(Args, "--");
        if (FlagsEnd != Args.begin()) {
          ClangCLMode =
              llvm::sys::path::stem(Args[0]).contains_insensitive("clang-cl") ||
              llvm::is_contained(Args, "--driver-mode=cl");

          // Reverse scan, starting at the end or at the element before "--".
          auto R = std::make_reverse_iterator(FlagsEnd);
          for (auto I = R, E = Args.rend(); I != E; ++I) {
            StringRef Arg = *I;
            if (ClangCLMode) {
              // Ignore arguments that are preceded by "-Xclang".
              if ((I + 1) != E && I[1] == "-Xclang")
                continue;
              if (LastO.empty()) {
                // With clang-cl, the output obj file can be specified with
                // "/opath", "/o path", "/Fopath", and the dash counterparts.
                // Also, clang-cl adds ".obj" extension if none is found.
                if ((Arg == "-o" || Arg == "/o") && I != R)
                  LastO = I[-1]; // Next argument (reverse iterator)
                else if (Arg.startswith("/Fo") || Arg.startswith("-Fo"))
                  LastO = Arg.drop_front(3).str();
                else if (Arg.startswith("/o") || Arg.startswith("-o"))
                  LastO = Arg.drop_front(2).str();

                if (!LastO.empty() && !llvm::sys::path::has_extension(LastO))
                  LastO.append(".obj");
              }
            }
            if (Arg == "-resource-dir")
              HasResourceDir = true;
          }
        }
        tooling::CommandLineArguments AdjustedArgs(Args.begin(), FlagsEnd);
        // The clang-cl driver passes "-o -" to the frontend. Inject the real
        // file here to ensure "-MT" can be deduced if need be.
        if (ClangCLMode && !LastO.empty()) {
          AdjustedArgs.push_back("/clang:-o");
          AdjustedArgs.push_back("/clang:" + LastO);
        }

        if (!HasResourceDir && ResourceDirRecipe == RDRK_InvokeCompiler) {
          StringRef ResourceDir =
              ResourceDirCache.findResourceDir(Args, ClangCLMode);
          if (!ResourceDir.empty()) {
            AdjustedArgs.push_back("-resource-dir");
            AdjustedArgs.push_back(std::string(ResourceDir));
          }
        }
        AdjustedArgs.insert(AdjustedArgs.end(), FlagsEnd, Args.end());
        return AdjustedArgs;
      });

  SharedStream Errs(llvm::errs());
  // Print out the dependency results to STDOUT by default.
  SharedStream DependencyOS(llvm::outs());

  DependencyScanningService Service(ScanMode, Format, OptimizeArgs,
                                    EagerLoadModules);
  llvm::ThreadPool Pool(llvm::hardware_concurrency(NumThreads));
  std::vector<std::unique_ptr<DependencyScanningTool>> WorkerTools;
  for (unsigned I = 0; I < Pool.getThreadCount(); ++I)
    WorkerTools.push_back(std::make_unique<DependencyScanningTool>(Service));

  std::vector<tooling::CompileCommand> Inputs =
      AdjustingCompilations->getAllCompileCommands();

  std::atomic<bool> HadErrors(false);
  FullDeps FD;
  P1689Deps PD;
  std::mutex Lock;
  size_t Index = 0;

  if (Verbose) {
    llvm::outs() << "Running clang-scan-deps on " << Inputs.size()
                 << " files using " << Pool.getThreadCount() << " workers\n";
  }
  for (unsigned I = 0; I < Pool.getThreadCount(); ++I) {
    Pool.async([I, &Lock, &Index, &Inputs, &HadErrors, &FD, &PD, &WorkerTools,
                &DependencyOS, &Errs]() {
      llvm::StringSet<> AlreadySeenModules;
      while (true) {
        const tooling::CompileCommand *Input;
        std::string Filename;
        std::string CWD;
        size_t LocalIndex;
        // Take the next input.
        {
          std::unique_lock<std::mutex> LockGuard(Lock);
          if (Index >= Inputs.size())
            return;
          LocalIndex = Index;
          Input = &Inputs[Index++];
          Filename = std::move(Input->Filename);
          CWD = std::move(Input->Directory);
        }
        std::optional<StringRef> MaybeModuleName;
        if (!ModuleName.empty())
          MaybeModuleName = ModuleName;

        std::string OutputDir(ModuleFilesDir);
        if (OutputDir.empty())
          OutputDir = getModuleCachePath(Input->CommandLine);
        auto LookupOutput = [&](const ModuleID &MID, ModuleOutputKind MOK) {
          return ::lookupModuleOutput(MID, MOK, OutputDir);
        };

        // Run the tool on it.
        if (Format == ScanningOutputFormat::Make) {
          auto MaybeFile = WorkerTools[I]->getDependencyFile(
              Input->CommandLine, CWD, MaybeModuleName);
          if (handleMakeDependencyToolResult(Filename, MaybeFile, DependencyOS,
                                             Errs))
            HadErrors = true;
        } else if (Format == ScanningOutputFormat::P1689) {
          // It is useful to generate the make-format dependency output during
          // the scanning for P1689. Otherwise the users need to scan again for
          // it. We will generate the make-format dependency output if we find
          // `-MF` in the command lines.
          std::string MakeformatOutputPath;
          std::string MakeformatOutput;

          auto MaybeRule = WorkerTools[I]->getP1689ModuleDependencyFile(
              *Input, CWD, MakeformatOutput, MakeformatOutputPath);
          HadErrors =
              handleP1689DependencyToolResult(Filename, MaybeRule, PD, Errs);

          if (!MakeformatOutputPath.empty() && !MakeformatOutput.empty() &&
              !HadErrors) {
            static std::mutex Lock;
            // With compilation database, we may open different files
            // concurrently or we may write the same file concurrently. So we
            // use a map here to allow multiple compile commands to write to the
            // same file. Also we need a lock here to avoid data race.
            static llvm::StringMap<llvm::raw_fd_ostream> OSs;
            std::unique_lock<std::mutex> LockGuard(Lock);

            auto OSIter = OSs.find(MakeformatOutputPath);
            if (OSIter == OSs.end()) {
              std::error_code EC;
              OSIter = OSs.try_emplace(MakeformatOutputPath,
                                       MakeformatOutputPath, EC)
                           .first;
              if (EC)
                llvm::errs()
                    << "Failed to open P1689 make format output file \""
                    << MakeformatOutputPath << "\" for " << EC.message()
                    << "\n";
            }

            SharedStream MakeformatOS(OSIter->second);
            llvm::Expected<std::string> MaybeOutput(MakeformatOutput);
            HadErrors = handleMakeDependencyToolResult(Filename, MaybeOutput,
                                                       MakeformatOS, Errs);
          }
        } else if (DeprecatedDriverCommand) {
          auto MaybeFullDeps =
              WorkerTools[I]->getFullDependenciesLegacyDriverCommand(
                  Input->CommandLine, CWD, AlreadySeenModules, LookupOutput,
                  MaybeModuleName);
          if (handleFullDependencyToolResult(Filename, MaybeFullDeps, FD,
                                             LocalIndex, DependencyOS, Errs))
            HadErrors = true;
        } else {
          auto MaybeFullDeps = WorkerTools[I]->getFullDependencies(
              Input->CommandLine, CWD, AlreadySeenModules, LookupOutput,
              MaybeModuleName);
          if (handleFullDependencyToolResult(Filename, MaybeFullDeps, FD,
                                             LocalIndex, DependencyOS, Errs))
            HadErrors = true;
        }
      }
    });
  }
  Pool.wait();

  if (Format == ScanningOutputFormat::Full)
    FD.printFullOutput(llvm::outs());
  else if (Format == ScanningOutputFormat::P1689)
    PD.printDependencies(llvm::outs());

  return HadErrors;
}
