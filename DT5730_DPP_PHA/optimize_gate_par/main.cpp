#include "Sort.h"

#include <iostream>
#include <fstream>
#include <assert.h>
#include <stdlib.h>

#include "TString.h"

int main(int argc, char const *argv[])
{
  if(argc != 6){
    std::cout << "need parameter" << std::endl;
    std::cout << "./sort run_num lg_start lg_stop sg_start sg_stop" << std::endl;
    std::cout << "like: ./sort 3 500 2000 700 1800" << std::endl;
    return -1;
  }

  int run = atoi(argv[1]);
  int lg_start = atoi(argv[2]);
  int lg_stop = atoi(argv[3]);
  int sg_start = atoi(argv[4]);
  int sg_stop = atoi(argv[5]);

  TString file_in = TString::Format("../raw2root/run%04d.root", run);
  std::cout << "sort " << file_in << std::endl;
  
  TString file_out = TString::Format("./rootfile/run%04d_lg%dlg%d_sg%dsg%d.root", run, lg_start, lg_stop, sg_start, sg_stop);

  Sort *so = new Sort(file_in.Data(), file_out.Data(), lg_start, lg_stop, sg_start, sg_stop);
  so->Process();

  delete so;

  return 0;
}
