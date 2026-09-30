#include <iostream>
#include <vector>
#include <thread>
#include <sstream>
#include <future>

#include "threadfuncs.h"

int main() {
  about();

  // Open log file
  Logger logger("output.log");

  std::cout << "main: pid = " << getThreadID()
            << ", opened file: 'output.log'\n";
  std::thread t_producer(producer, std::ref(logger));
  std::thread t_consumer(consumer, std::ref(logger));

  t_producer.join();
  t_consumer.join();

  std::cout << "Main: All threads finished.\n";
  // args for threads
  std::vector<ThreadArgs> args = {
    {1, "First"},
    {2, "Second"},
    {3, "Third"},
    {4, "Fourth"},
  };

  // thread are starting
  std::vector<std::thread> threads;
  std::vector<std::future<std::string>> futures;
  threads.reserve(COUNT_THREADS);

  for (int i = 0; i < COUNT_THREADS; ++i) {
    ThreadArgs args{i, "T" + std::to_string(i)};
    std::promise<std::string> prom;
    std::future<std::string> fut = prom.get_future();
    
   threads.emplace_back(funcThread, std::ref(args), std::ref(logger), std::move(prom));
   futures.push_back(std::move(fut));
  }
  for (auto& fut: futures){
    std::string result = fut.get();
    std::cout << "Result from thread: " << result << std::endl;
  }
  // wait for stop all threads
  for (auto& t : threads) {
    t.join();
  }

  // close file automatically
  logger.writeLine("output.log");
  
  return 0;
}
