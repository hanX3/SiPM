#include "Sort.h"
#include <fstream>

//
Sort::Sort(const std::string &filename_in, const std::string &filename_out)
{
  benchmark.reset(new TBenchmark);

  if(QDC_LONG_START < 0 || QDC_LONG_START >= QDC_LONG_STOP || QDC_SHORT_START < 0 || QDC_SHORT_START >= QDC_SHORT_STOP ||
     QDC_LONG_STOP > MAX_SAMPLES || QDC_SHORT_STOP > MAX_SAMPLES)
    throw std::runtime_error("invalid integration gates");
  GetCaliPar(par);
  GetTSOffset(ts_offset);

  file_in.reset(TFile::Open(filename_in.c_str()));
  if(!file_in || file_in->IsZombie())
    throw std::runtime_error("cannot open " + filename_in);

  file_out.reset(TFile::Open(filename_out.c_str(), "CREATE"));
  if(!file_out || file_out->IsZombie())
    throw std::runtime_error("cannot create " + filename_out + " (already exists?)");
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
  tr_out->Branch("v_data", &v_data);
  tr_out->Branch("v_dt", &v_dt);

  total_entry = 0;

  //
  memset(&data_raw, 0, sizeof(data_raw));
  size = 0;
  memset(&data, 0, sizeof(data));
  memset(&dt, 0, sizeof(dt));
  memset(&data_ana, 0, sizeof(data_ana));
  
  //

#ifdef DEBUG_SORT
  for(int i=0;i<MAX_CHANNELS;i++){
    std::cout << par[i][0] << " " << par[i][1] << " " << par[i][2] << std::endl;
  }
#endif

}

//
Sort::~Sort()
{
  // File and benchmark ownership is managed by unique_ptr.
}

//
void Sort::Process()
{
  benchmark->Start("sort");

  //
  Short_t channel;
  UShort_t energy;
  Long64_t timestamp;

  Long64_t nentries[MAX_CHANNELS] = {};
  TTree *tr[MAX_CHANNELS] = {};

  for(int i=0;i<MAX_CHANNELS;i++){
    tr[i] = dynamic_cast<TTree*>(file_in->Get(TString::Format("tr_ch%02d",i).Data()));
    if(!tr[i]){
#ifdef DEBUG_SORT
      std::cout << "channel " << i << " empty tree" << std::endl;
#endif
      nentries[i] = 0;
      continue;
    }

    total_entry += tr[i]->GetEntries();
    nentries[i] = tr[i]->GetEntries();

    if(tr[i]->SetBranchAddress("board", &data_raw.board) < 0 ||
       tr[i]->SetBranchAddress("channel", &channel) < 0 ||
       tr[i]->SetBranchAddress("energy_ch", &energy) < 0 ||
       tr[i]->SetBranchAddress("timestamp", &timestamp) < 0 ||
       tr[i]->SetBranchAddress("size", &size) < 0 ||
       tr[i]->SetBranchAddress("data", data) < 0 ||
       tr[i]->SetBranchAddress("dt", dt) < 0)
      throw std::runtime_error("missing or incompatible input branch");
  }

#ifdef DEBUG_SORT
  std::cout << "total_entry " << total_entry << std::endl;
#endif

  if(total_entry == 0) throw std::runtime_error("input contains no events");
  std::vector<Long64_t> ts(total_entry);
  std::vector<Long64_t> ts_ch(total_entry);
  
  total_entry = 0;
  for(int i=0;i<MAX_CHANNELS;i++){
    if(!tr[i]) continue;
    for(Long64_t j=0;j<tr[i]->GetEntries();j++){
      // Read the length separately, before ROOT writes into the fixed arrays.
      if(tr[i]->GetBranch("size")->GetEntry(j) <= 0 ||
         size > MAX_SAMPLES || size < BASELINE_SAMPLE ||
         size < static_cast<UInt_t>(QDC_LONG_STOP) || size < static_cast<UInt_t>(QDC_SHORT_STOP))
        throw std::runtime_error("waveform length incompatible with buffer or integration gates");
      if(tr[i]->GetBranch("timestamp")->GetEntry(j) <= 0)
        throw std::runtime_error("cannot read timestamp");
      ts[total_entry] = sipm::shift_timestamp(timestamp, ts_offset[i]);
      total_entry++;
    }
  }

  std::cout << "start sort ..." << std::endl;
  TMath::Sort(total_entry, ts.data(), ts_ch.data(), kFALSE);
#ifdef DEBUG_SORT
  for(int i=0;i<10;i++){
    std::cout << "ts " << ts[i] << " ts_ch " << ts_ch[i] << std::endl;
  }
#endif

  ts.clear();
  ts.shrink_to_fit();
  
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
    if(tr[tr_ch]->GetEntry(tr_entry) <= 0 || channel != tr_ch)
      throw std::runtime_error("unreadable event or channel/tree mismatch");
    data_raw.channel = channel;
    data_raw.energy = par[channel][0]+par[channel][1]*(Double_t)energy+par[channel][2]*(Double_t)energy*(Double_t)energy;
    data_raw.timestamp = sipm::shift_timestamp(timestamp, ts_offset[tr_ch]);

    //ana
    GetBaseline();
    GetAmplitudeMax();

    for(UInt_t k=0;k<size;k++){
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
  for(UInt_t i=0;i<size;i++){
    data_ana.energy_qdc += v_data[i];
  }
}

//
void Sort::GetQDCLong()
{
  data_ana.qdc_long = 0.;
  for(int i=QDC_LONG_START;i<QDC_LONG_STOP;i++){
    data_ana.qdc_long += v_data[i];
  }
  
  data_ana.qdc_long /= (Double_t)(QDC_LONG_STOP-QDC_LONG_START);
}

//
void Sort::GetQDCShort()
{
  data_ana.qdc_short = 0.;
  for(int i=QDC_SHORT_START;i<QDC_SHORT_STOP;i++){
    data_ana.qdc_short += v_data[i];
  }
  
  data_ana.qdc_short /= (Double_t)(QDC_SHORT_STOP-QDC_SHORT_START);
}

//
void GetCaliPar(Double_t p[MAX_CHANNELS][3])
{
  const auto rows = sipm::read_parameters(LABR3_CALI_DATA, MAX_CHANNELS, 4);
  for(int channel = 0; channel < MAX_CHANNELS; ++channel)
    for(int coefficient = 0; coefficient < 3; ++coefficient)
      p[channel][coefficient] = rows[channel * 4 + coefficient];
}

void GetTSOffset(Double_t p[MAX_CHANNELS])
{
  const auto rows = sipm::read_parameters(LABR3_TS_OFFSET_DATA, MAX_CHANNELS, 1);
  for(int channel = 0; channel < MAX_CHANNELS; ++channel) {
    sipm::shift_timestamp(0, rows[channel]);
    p[channel] = rows[channel];
  }
}
