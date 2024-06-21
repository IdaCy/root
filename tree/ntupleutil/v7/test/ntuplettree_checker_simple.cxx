#include <gtest/gtest.h>
#include "ROOT/RNTupleTTreeChecker.hxx"
#include "ROOT/RNTupleTTreeCheckerCLI.hxx"

#include <TFile.h>
#include <TTree.h>
#include <ROOT/RNTuple.hxx>
#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTupleWriter.hxx>

void createSimpleTTree(const char *filename, const char *treeName) {
   TFile file(filename, "RECREATE");
   TTree tree(treeName, "simple tree");

   int value = 0;
   tree.Branch("value", &value, "value/I");

   for (int i = 0; i < 100; ++i) {
      value = i;
      tree.Fill();
   }

   tree.Write();
}

void createSimpleRNTuple(const char *filename, const char *rntupleName) {
   std::remove(filename);
   auto model = ROOT::Experimental::RNTupleModel::Create();
   auto fieldValue = model->MakeField<int>("value");
   auto writer = ROOT::Experimental::RNTupleWriter::Recreate(std::move(model), rntupleName, filename);

   for (int i = 0; i < 100; ++i) {
      *fieldValue = i;
      writer->Fill();
   }
}

TEST(RNTupleTTreeChecker, SimpleComparison) {
   const char *ttreeFile = "simple_tree.root";
   const char *rntupleFile = "simple_rntuple.root";
   const char *treeName = "simple_tree";
   const char *rntupleName = "simple_rntuple";

   createSimpleTTree(ttreeFile, treeName);
   createSimpleRNTuple(rntupleFile, rntupleName);

   ROOT::Experimental::RNTupleTTreeCheckerCLI::CheckerConfig config;
   config.fTTreeFile = ttreeFile;
   config.fRNTupleFile = rntupleFile;
   config.fTTreeName = treeName;
   config.fRNTupleName = rntupleName;
   config.fShouldRun = true;

   ROOT::Experimental::RNTupleTTreeChecker checker;
   checker.Compare(config);

   std::remove(ttreeFile);
   std::remove(rntupleFile);
}
