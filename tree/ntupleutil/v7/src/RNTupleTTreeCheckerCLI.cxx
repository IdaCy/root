/// \file RNTupleTTreeCheckerCLI.cxx
/// \ingroup NTuple ROOT7
/// \author Ida Caspary <ida.friederike.caspary@cern.ch>
/// \date 2024-10-14
/// \warning This is part of the ROOT 7 prototype! It will change without notice. It might trigger earthquakes. Feedback
/// is welcome!

/*************************************************************************
 * Copyright (C) 1995-2023, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "ROOT/RNTupleTTreeCheckerCLI.hxx"
#include <iostream>
#include <vector>
#include <string>

namespace ROOT {
namespace Experimental {
namespace RNTupleTTreeCheckerCLI {

// Define usageText as a static constant within the namespace
static const char* usageText = "Usage:\n"
                               " rntuplettreechecker (--ttree|-t) <input_ttree_file>\n"
                               "                     (--rntuple|-r) <input_rntuple_file>\n"
                               "                     (--treename|-tn) <ttree_name>\n"
                               "                     (--rntuplename|-rn) <rntuple_name>\n"
                               " rntuplettreechecker [--help|-h]\n\n";

CheckerConfig ParseArgs(const std::vector<std::string>& args) {
    std::cout << "Total arguments received: " << args.size() << std::endl;
    for (const auto& arg : args) {
        std::cout << "Arg: " << arg << std::endl;
    }

    const auto argsProvided = args.size() >= 2;
    const auto helpUsed = argsProvided && (args[1] == "--help" || args[1] == "-h");

    if (!argsProvided || helpUsed) {
        std::cout << usageText;
        if (!argsProvided)
            std::cout << "Use --help or -h for usage help.";
        std::cout << std::endl;
        return {};
    }

    CheckerConfig config;

    for (size_t i = 1; i < args.size(); ++i) {
        const auto& arg = args[i];
        std::cout << "Processing argument " << i << ": " << arg << std::endl;

        if (arg == "--ttree" || arg == "-t") {
            if (++i < args.size()) {
                config.fTTreeFile = args[i];
                std::cout << "TTree file set to: " << config.fTTreeFile << std::endl;
            }
        } else if (arg == "--rntuple" || arg == "-r") {
            if (++i < args.size()) {
                config.fRNTupleFile = args[i];
                std::cout << "RNTuple file set to: " << config.fRNTupleFile << std::endl;
            }
        } else if (arg == "--treename" || arg == "-tn") {
            if (++i < args.size()) {
                config.fTTreeName = args[i];
                std::cout << "Tree name set to: " << config.fTTreeName << std::endl;
            }
        } else if (arg == "--rntuplename" || arg == "-rn") {
            if (++i < args.size()) {
                config.fRNTupleName = args[i];
                std::cout << "RNTuple name set to: " << config.fRNTupleName << std::endl;
            }
        } else {
            std::cerr << "Unknown argument '" << arg << "'\n" << usageText << "\n";
            return {};
        }
    }

    config.fShouldRun = true;
    return config;
}

} // namespace RNTupleTTreeCheckerCLI
} // namespace Experimental
} // namespace ROOT
