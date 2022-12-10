#ifndef SORT_H_
#define SORT_H_

#include "set.h"
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <string.h> //memset()
#include <stdlib.h> //malloc()

#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TMath.h"
#include "TBenchmark.h"

void GetCaliPar(Double_t p[MAX_CHANNELS][3]);
void GetTSOffset(Double_t p[MAX_CHANNELS]);

typedef struct {
  Short_t board;
  Short_t channel;
  Double_t energy;
  Long64_t timestamp;
} Data_Raw;

typedef struct {
  Double_t baseline;
  Double_t amplitude_max;
  Double_t energy_qdc;
  Double_t qdc_long;
  Double_t qdc_short;
} Data_Ana;

//
class Sort
{
public:
  Sort() = default;
  Sort(const std::string &filename_in, const std::string &filename_out, const int lg1, const int lg2, const int sg1, const int sg2);
  ~Sort();

public:
  void Process();

public:
  void GetBaseline();
  void GetAmplitudeMax();
  void GetEnergyQDC();
  void GetQDCLong();
  void GetQDCShort();

private:
  int lg_start;
  int lg_stop;
  int sg_start;
  int sg_stop;

private:
  Double_t par[MAX_CHANNELS][3];
  Double_t ts_offset[MAX_CHANNELS];

private:
  Long64_t total_entry;

  Data_Raw data_raw;
  UInt_t size;
  Short_t data[MAX_SAMPLES];
  Short_t dt[MAX_SAMPLES];

private:
  Data_Ana data_ana;
  std::vector<Double_t> v_data;
  std::vector<Double_t> v_dt;

private:
  TBenchmark *benchmark;
  TFile *file_in;
  TFile *file_out;
  TTree *tr_out;
};

#endif
