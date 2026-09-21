#include <kan/core.hpp>

#include <ctime>
#include <fstream>
#include <iomanip>

void logMessage(const std::string &msg, const std::string &file) {
  std::time_t   t = std::time(nullptr);
  std::ofstream out(file, std::ios::app);
  out << std::put_time(std::localtime(&t), "%F %T") << "  " << msg << '\n';
}
