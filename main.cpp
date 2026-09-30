#include <iostream>
#include <vector>
#include <thread>
#include <sstream>

#include "threadfuncs.h"

int main() {
  about();

  // Open log file
  Logger logger("output.log");

  std::cout << "main: pid = " << getThreadID()
            << ", opened file: 'output.log'\n";

  // args for threads
  std::vector<ThreadArgs> args = {
    {1, "First"},
    {2, "Second"},
    {3, "Third"},
    {4, "Fourth"},
  };

  // thread are starting
  std::vector<std::thread> threads;
  threads.reserve(COUNT_THREADS);

  for (int i = 0; i < COUNT_THREADS; ++i) {
    std::ostringstream oss;
    oss << "T" << i;
    args[i].id = i;
    args[i].tag = oss.str();
  }

  // wait for stop all threads
  for (auto& t : threads) {
    if (t.joinable()) t.join();
  }

  // close file automatically
  logger.writeLine("output.log");
  return 0;
}
