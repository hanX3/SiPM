#ifndef SIPM_INPUT_CHECKS_H
#define SIPM_INPUT_CHECKS_H

#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

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

// EOF is valid only before the first field of a new event.
inline bool read_exact(FILE *file, void *destination, size_t bytes, bool allow_eof = false)
{
  const size_t count = std::fread(destination, 1, bytes, file);
  if(count == bytes) return true;
  if(allow_eof && count == 0 && std::feof(file) && !std::ferror(file)) return false;
  throw std::runtime_error("truncated event or read error in binary input");
}

// One complete row per channel. A final newline is optional; # starts a comment.
inline std::vector<double> read_parameters(const char *path, int channels, int columns)
{
  std::ifstream input(path);
  if(!input) throw std::runtime_error(std::string("cannot open parameter file: ") + path);
  std::vector<double> values(channels * columns);
  std::vector<bool> seen(channels, false);
  std::string line;
  while(std::getline(input, line)) {
    line = line.substr(0, line.find('#'));
    std::istringstream row(line);
    row >> std::ws;
    if(row.eof()) continue;
    int channel;
    if(!(row >> channel) || channel < 0 || channel >= channels || seen[channel])
      throw std::runtime_error(std::string("invalid or duplicate channel in ") + path);
    for(int column = 0; column < columns; ++column) {
      double value;
      if(!(row >> value) || !std::isfinite(value))
        throw std::runtime_error(std::string("invalid parameter row in ") + path);
      values[channel * columns + column] = value;
    }
    std::string extra;
    if(row >> extra) throw std::runtime_error(std::string("extra parameter in ") + path);
    seen[channel] = true;
  }
  if(input.bad()) throw std::runtime_error(std::string("read error in ") + path);
  for(int channel = 0; channel < channels; ++channel)
    if(!seen[channel]) throw std::runtime_error(std::string("missing channel in ") + path);
  return values;
}

inline long long shift_timestamp(long long timestamp, double offset)
{
  const long double value = offset;
  if(!std::isfinite(offset) || std::trunc(offset) != offset ||
     value < std::numeric_limits<long long>::min() ||
     value > std::numeric_limits<long long>::max())
    throw std::runtime_error("timestamp offset must be an integer number of ps");
  const long long delta = static_cast<long long>(offset);
  if((delta > 0 && timestamp > std::numeric_limits<long long>::max() - delta) ||
     (delta < 0 && timestamp < std::numeric_limits<long long>::min() - delta))
    throw std::runtime_error("timestamp overflow");
  return timestamp + delta;
}

} // namespace sipm
#endif
