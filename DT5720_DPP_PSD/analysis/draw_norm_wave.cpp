#include <vector>
#include <algorithm>

//
void draw_norm_wave(int run_num)
{
  gROOT->SetBatch(0);

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
  Double_t dt[size];
  Double_t data_nom[size];
  memset(&data_nom, 0, sizeof(data_nom));

  TGraph *gr[1000];
  TMultiGraph *mg = new TMultiGraph();

  for(Long64_t i=0;i<tr->GetEntries();i++){
    if(i>999) break;

    tr->GetEntry(i);

    if(amplitude_max<400) continue;

    
    for(UInt_t j=0;j<v_data->size();++j){
      dt[j] = j;
      data_nom[j] = v_data->at(j)/amplitude_max + 0.2*(gRandom->Rndm()-0.5);
    }

    gr[i] = new TGraph(size, dt, data_nom);

    if(i==0) gr[i]->Draw();
    else gr[i]->Draw("same");
  }

  return;
}

//
void draw_norm_wave_multirun(int run_num1, int run_num2)
{
  gROOT->SetBatch(0);

  TFile *fi1 = TFile::Open(TString::Format("../sort/run%04d_sort.root",run_num1).Data());
  if(fi1->IsZombie()){
    cout << "wrong open the file" << endl;
    return;
  }
  TTree *tr1 = (TTree*)fi1->Get("tr");

  TFile *fi2 = TFile::Open(TString::Format("../sort/run%04d_sort.root",run_num2).Data());
  if(fi2->IsZombie()){
    cout << "wrong open the file" << endl;
    return;
  }
  TTree *tr2 = (TTree*)fi2->Get("tr");
  

  //
  Double_t amplitude_max2;
  std::vector<double> *v_data2 = 0;

  tr2->SetBranchAddress("amplitude_max", &amplitude_max2);
  tr2->SetBranchAddress("v_data", &v_data2);
  tr2->GetEntry(0);
  UInt_t size2 = v_data2->size();
  Double_t dt2[size2];
  Double_t data_nom2[size2];
  memset(&data_nom2, 0, sizeof(data_nom2));

  TGraph *gr2[1000];

  for(Long64_t i=0;i<tr2->GetEntries();i++){
    if(i>999) break;

    tr2->GetEntry(i);

    if(amplitude_max2<100) continue;
    
    for(UInt_t j=0;j<v_data2->size();++j){
      dt2[j] = j;
      data_nom2[j] = v_data2->at(j)/amplitude_max2 + 0.2*(gRandom->Rndm()-0.5);
    }

    gr2[i] = new TGraph(size2, dt2, data_nom2);
    gr2[i]->SetLineColor(4);

    if(i==0) gr2[i]->Draw();
    else gr2[i]->Draw("same");
  }

  //
  Double_t amplitude_max1;
  std::vector<double> *v_data1 = 0;

  tr1->SetBranchAddress("amplitude_max", &amplitude_max1);
  tr1->SetBranchAddress("v_data", &v_data1);
  tr1->GetEntry(0);
  UInt_t size1 = v_data1->size();
  Double_t dt1[size1];
  Double_t data_nom1[size1];
  memset(&data_nom1, 0, sizeof(data_nom1));

  TGraph *gr1[1000];

  for(Long64_t i=0;i<tr1->GetEntries();i++){
    if(i>999) break;

    tr1->GetEntry(i);

    if(amplitude_max1<400) continue;
    
    for(UInt_t j=0;j<v_data1->size();++j){
      dt1[j] = j;
      data_nom1[j] = v_data1->at(j)/amplitude_max1 + 0.2*(gRandom->Rndm()-0.5);
    }

    gr1[i] = new TGraph(size1, dt1, data_nom1);
    gr1[i]->SetLineColor(2);  

    gr1[i]->Draw("same");
  }

  

  return;
}