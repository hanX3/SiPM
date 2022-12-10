#include "Sort.h"

#include <iostream>
#include <fstream>
#include <assert.h>
#include <stdlib.h>

#include "TString.h"

int main(int argc, char const *argv[])
{
  if(argc != 2){
    std::cout << "need parameter" << std::endl;
    std::cout << "like: sort 33" << std::endl;
    return -1;
  }

  int run = atoi(argv[1]);
  TString file_in = TString::Format("../raw2root/run%04d.root", run);
  std::cout << "sort " << file_in << std::endl;
  
  TString file_out = TString::Format("./run%04d_sort.root", run);

  Sort *so = new Sort(file_in.Data(), file_out.Data());
  so->Process();

  delete so;

  return 0;
}
