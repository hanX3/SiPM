#define fit_test_cxx
#include "fit_test.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>

void fit_test::Loop()
{
//   In a ROOT session, you can do:
//      root> .L fit_test.C
//      root> fit_test t
//      root> t.GetEntry(12); // Fill t data members with entry number 12
//      root> t.Show();       // Show values of entry 12
//      root> t.Show(16);     // Read and show values of entry 16
//      root> t.Loop();       // Loop on all entries
//

//     This is the loop skeleton where:
//    jentry is the global entry number in the chain
//    ientry is the entry number in the current Tree
//  Note that the argument to GetEntry must be:
//    jentry for TChain::GetEntry
//    ientry for TTree::GetEntry and TBranch::GetEntry
//
//       To read only selected branches, Insert statements like:
// METHOD1:
//    fChain->SetBranchStatus("*",0);  // disable all branches
//    fChain->SetBranchStatus("branchname",1);  // activate branchname
// METHOD2: replace line
//    fChain->GetEntry(jentry);       //read all branches
//by  b_branchname->GetEntry(ientry); //read only this branch
   if (fChain == 0) return;

   Long64_t nentries = fChain->GetEntriesFast();

   Long64_t nbytes = 0, nb = 0;
   for (Long64_t jentry=0; jentry<nentries;jentry++) {
      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0) break;
      nb = fChain->GetEntry(jentry);   nbytes += nb;
      // if (Cut(ientry) < 0) continue;
   }
}

//
void fit_test::FitTail(Long64_t entry)
{
   fChain->GetEntry(entry);

   TH1D *h1 = new TH1D("h1", "", size, 0, size);
   for(int i=0;i<size;i++){
      h1->SetBinContent(i+1, data[i]);
   }

   h1->Draw();

   TF1 *tf = new TF1("tf", "FittfTau", 1200, 2400, 3);
   tf->SetParameter(0, data[0]);
   tf->SetParameter(1, data[500]);
   tf->SetParameter(2, 3000.);

   h1->Fit(tf, "R");
}

//
void fit_test::FitWave(Long64_t entry)
{
   fChain->GetEntry(entry);

   TH1D *h1 = new TH1D("h1", "", size, 0, size);
   for(int i=0;i<size;i++){
      h1->SetBinContent(i+1, data[i]-data[0]);
   }

   h1->Draw();

   TF1 *tf_wave = new TF1("tf_wave", "FittfWave", 0, 2400, 6);
   tf_wave->FixParameter(0, 0);
   // tf_wave->SetParameter(0, data[0]);
   tf_wave->FixParameter(1, 400);
   tf_wave->SetParameter(2, 800);
   tf_wave->SetParameter(3, 3500);
   tf_wave->SetParameter(4, data[500]/2.);
   tf_wave->SetParameter(5, data[500]/2.);

   tf_wave->SetParLimits(2, 200, 600);
   tf_wave->SetParLimits(3, 2000, 6000);

   h1->Fit(tf_wave, "WR");
}

//p[0]: baseline
//p[1]: amplitude
//p[2]: tau
Double_t FittfTau(Double_t *i, Double_t *p)
{
   Double_t s = p[0];
   s += p[1]*exp(-i[0]/p[2]);

   return s;
}

//p[0]: baseline
//p[1]: t0
//p[2]: tau_fast
//p[3]: tau_slow
//p[4]: amp_fast
//p[5]: amp_slow
Double_t FittfWave(Double_t *i, Double_t *p)
{
   Double_t s = p[0];
   Double_t x = i[0]-p[1];
   if(x<0) return s;
   else{
      s += p[4]*exp(-x/p[2]);
      s += p[5]*exp(-x/p[3]);
   }

   return s;
}
