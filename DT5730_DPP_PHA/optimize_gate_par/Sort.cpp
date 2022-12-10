#include "Sort.h"
#include <fstream>

//
Sort::Sort(const std::string &filename_in, const std::string &filename_out, const int lg1, const int lg2, const int sg1, const int sg2)
{
  benchmark = new TBenchmark;

  lg_start = lg1;
  lg_stop = lg2;
  sg_start = sg1;
  sg_stop = sg2;

  file_in = TFile::Open(filename_in.c_str());
  if(file_in->IsZombie()){
    std::cout << "cannot open " << filename_in << std::endl;
  }

  file_out = TFile::Open(filename_out.c_str(), "recreate");
  tr_out = new TTree("tr", "DT5730_DPP_PHA");

  tr_out->Branch("board", &data_raw.board, "board/S");
  tr_out->Branch("channel", &data_raw.channel, "channel/S");
  tr_out->Branch("energy", &data_raw.energy, "energy/D");
  tr_out->Branch("timestamp", &data_raw.timestamp, "timestamp/L");

  tr_out->Branch("baseline", &data_ana.baseline, "baseline/D");
  tr_out->Branch("amplitude_max", &data_ana.amplitude_max, "amplitude_max/D");
  tr_out->Branch("energy_qdc", &data_ana.energy_qdc, "energy_qdc/D");
  tr_out->Branch("qdc_long", &data_ana.qdc_long, "qdc_long/D");
  tr_out->Branch("qdc_short", &data_ana.qdc_short, "qdc_short/D");
  // tr_out->Branch("v_data", &v_data);
  // tr_out->Branch("v_dt", &v_dt);

  total_entry = 0;

  //
  memset(&data_raw, 0, sizeof(data_raw));
  size = 0;
  memset(&data, 0, sizeof(data));
  memset(&dt, 0, sizeof(dt));
  memset(&data_ana, 0, sizeof(data_ana));
  
  //
  memset(par, 0, sizeof(par)); 
  GetCaliPar(par);
  GetTSOffset(ts_offset);

#ifdef DEBUG_SORT
  for(int i=0;i<MAX_CHANNELS;i++){
    std::cout << par[i][0] << " " << par[i][1] << " " << par[i][2] << std::endl;
  }
#endif

}

//
Sort::~Sort()
{
  file_in->Close();
  file_out->Close();
}

//
void Sort::Process()
{
  benchmark->Start("sort");

  //
  Short_t channel;
  UShort_t energy;
  Long64_t timestamp;

  Long64_t nentries[MAX_CHANNELS];
  TTree *tr[MAX_CHANNELS];

  for(int i=0;i<MAX_CHANNELS;i++){
    tr[i] = (TTree*)file_in->Get(TString::Format("tr_ch%02d",i).Data());
    if(!tr[i]){
#ifdef DEBUG_SORT
      std::cout << "channel " << i << " empty tree" << std::endl;
#endif
      nentries[i] = 0;
      continue;
    }

    total_entry += tr[i]->GetEntries();
    nentries[i] = tr[i]->GetEntries();

    tr[i]->SetBranchAddress("channel", &channel);
    tr[i]->SetBranchAddress("energy_ch", &energy);
    tr[i]->SetBranchAddress("timestamp", &timestamp);
    tr[i]->SetBranchAddress("size", &size);
    tr[i]->SetBranchAddress("data", data);
    tr[i]->SetBranchAddress("dt", dt);
  }

#ifdef DEBUG_SORT
  std::cout << "total_entry " << total_entry << std::endl;
#endif

  Long64_t *ts;
  ts = (Long64_t*)malloc(total_entry*sizeof(Long64_t)); 
  Int_t *ts_ch;
  ts_ch = (Int_t*)malloc(total_entry*sizeof(Int_t));
  
  total_entry = 0;
  for(int i=0;i<MAX_CHANNELS;i++){
    if(!tr[i]) continue;
    for(Long64_t j=0;j<tr[i]->GetEntries();j++){
      tr[i]->GetEntry(j);
      // may add own cfd here, to do
      // ts[total_entry] = timestamp;
      ts[total_entry] = timestamp+ts_offset[i];
      total_entry++;
    }
  }

  std::cout << "start sort ..." << std::endl;
  TMath::Sort((Int_t)total_entry, (Long64_t*)ts, (Int_t*)ts_ch, kFALSE);
#ifdef DEBUG_SORT
  for(int i=0;i<10;i++){
    std::cout << "ts " << ts[i] << " ts_ch " << ts_ch[i] << std::endl;
  }
#endif

  free(ts);
  
  Long64_t min_tag[MAX_CHANNELS], max_tag[MAX_CHANNELS];
  memset(min_tag, 0, sizeof(min_tag));
  memset(max_tag, 0, sizeof(max_tag));
  for(int i=0;i<MAX_CHANNELS;i++){
    for(int j=0;j<i;j++)  min_tag[i] += nentries[j];
    for(int j=0;j<=i;j++)  max_tag[i] += nentries[j];
  }

  Int_t tr_ch = 0;
  Long64_t tr_entry = 0;

  for(Long64_t i=0;i<total_entry;i++){
    if(i%10000==0){
      std::cout << i << "/" << total_entry << std::endl;
    }
    for(int j=0;j<MAX_CHANNELS;j++){
      if(ts_ch[i]>=min_tag[j] && ts_ch[i]<max_tag[j]){
        tr_ch = j;
        tr_entry = ts_ch[i]-min_tag[j];
        break;
      }
    }
    tr[tr_ch]->GetEntry(tr_entry);
    data_raw.channel = channel;
    data_raw.energy = par[channel][0]+par[channel][1]*(Double_t)energy+par[channel][2]*(Double_t)(energy*energy);
    data_raw.timestamp = timestamp+ts_offset[tr_ch];

    //ana
    GetBaseline();
    GetAmplitudeMax();

    for(int k=0;k<size;k++){
      v_data.push_back((Double_t)data[k]-data_ana.baseline);
      v_dt.push_back(k);
    }

    GetEnergyQDC();
    GetQDCLong();
    GetQDCShort();

    // Fill tree
    tr_out->Fill();

#ifdef DEBUG_SORT
    if(i<10){
      std::cout << "board " << data_raw.board << std::endl;
      std::cout << "channel " << data_raw.channel << std::endl;
      std::cout << "energy " << data_raw.energy << std::endl;
      std::cout << "timestamp " << data_raw.timestamp << std::endl;

      std::cout << "size " << data_ana.baseline << std::endl;
      std::cout << "amplitude_max " << data_ana.amplitude_max << std::endl;
    }
#endif

    // clear
    memset(&data_raw, 0, sizeof(data_raw));
    size = 0;
    memset(&data, 0, sizeof(data));
    memset(&dt, 0, sizeof(dt));
    memset(&data_ana, 0, sizeof(data_ana));
    v_data.clear();
    v_dt.clear();
  }
  free(ts_ch);

#ifdef DEBUG_SORT
  std::cout << "vec_d size " << vec_d.size() << std::endl;
  for(int i=0;i<10;i++){
    std::cout << vec_d[i].channel << " " << vec_d[i].energy << " " << vec_d[i].timestamp << std::endl;
  }
#endif

  //
  file_out->cd();
  tr_out->Write();
  file_out->Close();

  benchmark->Show("sort");
}

//
void Sort::GetBaseline()
{
  data_ana.baseline = 0.;
  for(int i=0;i<BASELINE_SAMPLE;i++){
    data_ana.baseline += data[i];
  }

  data_ana.baseline /= (Double_t)BASELINE_SAMPLE;
}

//
void Sort::GetAmplitudeMax()
{
  data_ana.amplitude_max = (Double_t)*std::max_element(data, data+size);
  data_ana.amplitude_max -= data_ana.baseline;
}

//
void Sort::GetEnergyQDC()
{
  data_ana.energy_qdc = 0.;
  for(int i=0;i<size;i++){
    data_ana.energy_qdc += v_data[i];
  }
}

//
void Sort::GetQDCLong()
{
  data_ana.qdc_long = 0.;
  for(int i=lg_start;i<lg_stop;i++){
    data_ana.qdc_long += v_data[i];
  }
  
  data_ana.qdc_long /= (Double_t)(lg_stop-lg_start);
}

//
void Sort::GetQDCShort()
{
  data_ana.qdc_short = 0.;
  for(int i=sg_start;i<sg_stop;i++){
    data_ana.qdc_short += v_data[i];
  }
  
  data_ana.qdc_short /= (Double_t)(sg_stop-sg_start);
}

//
void GetCaliPar(Double_t p[MAX_CHANNELS][3])
{
  std::ifstream f;
  f.open(TString::Format("%s",LABR3_CALI_DATA).Data());
  if(!f){
    std::cout << "can not open " << LABR3_CALI_DATA << std::endl;
    return;
  }

  Int_t ch;
  Double_t par0, par1, par2;
  Double_t chi2;

  while(true){
    f >> ch >> par0 >> par1 >> par2 >> chi2;
    if(!f.good()) break;

    p[ch][0] = par0;
    p[ch][1] = par1;
    p[ch][2] = par2;
  }
  f.close();
}

//
void GetTSOffset(Double_t p[MAX_CHANNELS])
{
  std::ifstream f;
  f.open(TString::Format("%s",LABR3_TS_OFFSET_DATA).Data());
  if(!f){
    std::cout << "can not open " << LABR3_TS_OFFSET_DATA << std::endl;
    return;
  }

  Int_t ch;
  Double_t offset;

  while(true){
    f >> ch >> offset;
    if(!f.good()) break;

    p[ch] = offset;
  }
  f.close();
}
