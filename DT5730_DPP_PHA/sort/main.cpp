#include "Sort.h"

#include <iostream>
#include <fstream>
#include <assert.h>
#include <stdlib.h>

#include "TString.h"

#include <cerrno>
#include <cstdlib>
#include <limits>
#include <stdexcept>

namespace sipm {

inline int nonnegative_int(const char *text)
{
  char *end = nullptr;
  errno = 0;
  const long value = std::strtol(text, &end, 10);
  if(errno || end == text || *end || value < 0 || value > std::numeric_limits<int>::max())
    throw std::runtime_error(std::string("invalid nonnegative integer: ") + text);
  return static_cast<int>(value);
}

} // namespace sipm

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
