#define ana_cxx
#include "ana.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>

void ana::Loop()
{
//   In a ROOT session, you can do:
//      root> .L ana.C
//      root> ana t
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
void ana::DrawSingle(Int_t channel, Long64_t n)
{
  Long64_t i = 0;
  Long64_t id = 0;
  while(1){
    // cout << i << endl;
    Long64_t j = LoadTree(i);
    if(j<0){
      cout << "cannot open event" << endl;
      return;
    }
    fChain->GetEntry(i);

    if(channel!=ch){
      i++;
      continue;
    }else if(id!=n){
      i++;
      id++;
      continue;
    }else break;
  }

  TH1D *h = new TH1D("h", "", ltra, 0, ltra);
  for(int k=0;k<ltra;k++){
    h->SetBinContent(k+1, data[k]);
  }

  auto cc = new TCanvas();
  cc->cd();
  h->Draw();
}

//
void ana::DrawMulti(Int_t channel, Long64_t n)
{
  vector<double> v_data;

  Long64_t i = 0;
  Long64_t id = 0;
  while(1){
    cout << i << endl;
    Long64_t j = LoadTree(i);
    if(j<0){
      cout << "cannot open event" << endl;
      return;
    }
    fChain->GetEntry(i);

    if(channel!=ch){
      i++;
      continue;
    }else if(id==0){
      i++;
      id++;
      for(int k=0;k<ltra;k++){
        v_data.push_back(data[k]);
      }
      cout << v_data[0] << endl;
      continue;
    }else if(id!=n){
      i++;
      id++;
      for(int k=0;k<ltra;k++){
        v_data[k] += data[k];
      }
      cout << v_data[0] << endl;
    }else break;
  }

  TH1D *h = new TH1D("h", "", ltra, 0, ltra);
  for(int k=0;k<ltra;k++){
    h->SetBinContent(k+1, v_data[k]/(double)n);
  }

  auto cc = new TCanvas();
  cc->cd();
  h->Draw();

  ofstream fo;
  fo.open("wave.txt");
  for(int k=0;k<ltra;k++){
    fo << k << " " << v_data[k]/(double)n << endl;
  }
  fo.close();
}
