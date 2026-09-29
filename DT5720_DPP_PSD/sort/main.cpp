#include "Sort.h"

#include <iostream>
#include <fstream>
#include <assert.h>
#include <stdlib.h>

#include "TString.h"

int main(int argc, char const *argv[])
try {
  if(argc != 2){
    std::cout << "need parameter" << std::endl;
    std::cout << "like: sort 33" << std::endl;
    return -1;
  }

  int run = sipm::nonnegative_int(argv[1]);
  TString file_in = TString::Format("../raw2root/run%04d.root", run);
  std::cout << "sort " << file_in << std::endl;
  
  TString file_out = TString::Format("./run%04d_sort.root", run);

  Sort so(file_in.Data(), file_out.Data());
  so.Process();

  return 0;
}

catch(const std::exception &error) {
  std::cerr << "sort: " << error.what() << std::endl;
  return 1;
}
