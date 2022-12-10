#include <vector>
#include <algorithm>

void get_averge_wave(int run_num)
{
  gROOT->SetBatch(1);

  TFile *fi = TFile::Open(TString::Format("../sort/run%04d_sort.root",run_num).Data());
  if(fi->IsZombie()){
    cout << "wrong open the file" << endl;
    return;
  }

  TTree *tr = (TTree*)fi->Get("tr");
  
  Double_t amplitude_max;
  std::vector<double> *v_data = 0;

  tr->SetBranchAddress("amplitude_max", &amplitude_max);
  tr->SetBranchAddress("v_data", &v_data);

  //
  tr->GetEntry(0);
  UInt_t size = v_data->size();
  Double_t data_avg[size];
  Double_t data_avg_nom[size];
  memset(&data_avg, 0, sizeof(data_avg));
  memset(&data_avg_nom, 0, sizeof(data_avg_nom));

  Double_t n = 0;
  for(Long64_t i=0;i<tr->GetEntries();i++){
    tr->GetEntry(i);

    if(amplitude_max<30) continue;

    n++;
    for(UInt_t i=0;i<v_data->size();++i){
      data_avg[i] += v_data->at(i);
    }
  }

  for(int i=0;i<size;i++){
    data_avg[i] /= n;
  }

  //
  ofstream fo(TString::Format("./averge_wave_run%04d.txt",run_num).Data());

  Double_t data_avg_max = *std::max_element(data_avg, data_avg+size);
  for(int i=0;i<size;i++){
    fo << i << " " << data_avg[i] << " " << data_avg[i]/data_avg_max << endl;
  }

  fi->Close();
  fo.close();

  return;
}