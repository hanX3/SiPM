//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Sat Dec 10 22:02:09 2022 by ROOT version 6.24/02
// from TTree tr_ch00/DT5730_DPP_PHA
// found on file: ../raw2root/run0003.root
//////////////////////////////////////////////////////////

#ifndef fit_test_h
#define fit_test_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>



Double_t Fittf(Double_t *i, Double_t *p);

// Header file for the classes stored in the TTree if any.

class fit_test {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

// Fixed size dimensions of array or collections stored in the TTree if any.

   // Declaration of leaf types
   Short_t         board;
   Short_t         channel;
   Long64_t        timestamp;
   UShort_t        energy_ch;
   UInt_t          size;
   Short_t         data[2496];   //[size]
   Short_t         dt[2496];   //[size]
   Long64_t        n;

   // List of branches
   TBranch        *b_board;   //!
   TBranch        *b_channel;   //!
   TBranch        *b_timestamp;   //!
   TBranch        *b_energy_ch;   //!
   TBranch        *b_size;   //!
   TBranch        *b_data;   //!
   TBranch        *b_dt;   //!
   TBranch        *b_n;   //!

   fit_test(TTree *tree=0);
   virtual ~fit_test();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop();
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);

   //hanX
   void FitTail(Long64_t entry);
   void FitWave(Long64_t entry);
};

#endif

#ifdef fit_test_cxx
fit_test::fit_test(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("../raw2root/run0003.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("../raw2root/run0003.root");
      }
      f->GetObject("tr_ch00",tree);

   }
   Init(tree);
}

fit_test::~fit_test()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t fit_test::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t fit_test::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!fChain) return -5;
   Long64_t centry = fChain->LoadTree(entry);
   if (centry < 0) return centry;
   if (fChain->GetTreeNumber() != fCurrent) {
      fCurrent = fChain->GetTreeNumber();
      Notify();
   }
   return centry;
}

void fit_test::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("board", &board, &b_board);
   fChain->SetBranchAddress("channel", &channel, &b_channel);
   fChain->SetBranchAddress("timestamp", &timestamp, &b_timestamp);
   fChain->SetBranchAddress("energy_ch", &energy_ch, &b_energy_ch);
   fChain->SetBranchAddress("size", &size, &b_size);
   fChain->SetBranchAddress("data", data, &b_data);
   fChain->SetBranchAddress("dt", dt, &b_dt);
   fChain->SetBranchAddress("n", &n, &b_n);
   Notify();
}

Bool_t fit_test::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void fit_test::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t fit_test::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef fit_test_cxx
