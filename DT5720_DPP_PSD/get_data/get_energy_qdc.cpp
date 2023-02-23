
void get_energy_qdc(int run_num)
{
  gROOT->SetBatch(1);

  TFile *fi = TFile::Open(TString::Format("../sort/run%04d_sort.root",run_num).Data());
  if(fi->IsZombie()){
    cout << "wrong open the file" << endl;
    return;
  }

  TTree *tr = (TTree*)fi->Get("tr");
  TH1D *h1 = new TH1D("h1", "h1", 512, 0, 2048);

  tr->Draw("energy_qdc>>h1");

  ofstream fo(TString::Format("./energy_qdc_run%04d.txt",run_num).Data());
  for(int i=0;i<h1->GetNbinsX();i++){
    fo << i+1 << " " << h1->GetBinContent(i+1) << endl;
  }

  fi->Close();
  fo.close();

  return;
}