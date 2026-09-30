// threadfuncs.cpp
#include "threadfuncs.h"
#include <thread>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <syscall.h>
//#include <windows.h>
#include <sys/types.h>
#include <atomic>
#include <chrono>

std::atomic<int> counter{0};
Logger::Logger(const std::string& filename)
  : file_(filename, std::ios::out | std::ios::trunc)
{
  if (!file_.is_open()) {
    throw std::runtime_error("Cannot open log file: " + filename);
  }
}

Logger::~Logger() {
  // std::ofstream close file here automatically
}

bool Logger::writeLine(const std::string& msg) {
  std::lock_guard<std::mutex> lock(mutex_);
  file_ << msg;
  file_.flush();
  if (!file_) {
    std::cerr << "write failed: " << msg << "\n";
    return false;
  }
  return true;
}

pid_t getThreadID() {
  return static_cast<pid_t>(::syscall(SYS_gettid));
  //return GetCurrentThreadId();
}

void about() {
  std::cout << "std::thread example\n";
}

std::string funcThread(const ThreadArgs& args, Logger& logger, std::promise<std::string> prom) {
  for (int i = 0; i < COUNT_ITERATIONS; ++i) {
    std::ostringstream oss;

    oss << "[tag = " << args.tag
        << "] pid = "  << ::getpid()
        << " ppid = "  << ::getppid()
        << " tid = "   << getThreadID()
	<< " std::thread::id = " << std::this_thread::get_id()
        << " iter = "  << i
        << "\n";
    if(!logger.writeLine(oss.str())){
	std::cerr << "Error writting to log in thread" << args.id << "\n";
    }

    // imitation of useful work
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    counter++;
  }
  std::string result = "Thread " + args.tag + " completed" + std::to_string(COUNT_ITERATIONS) + " iterations";
  prom.set_value(result);
}

// Потребитель-производитель

std::mutex mtx;
std::condition_variable cv;
int shared_value = 0;
bool ready = false;
bool done = false;

void producer(Logger& logger){
  for (int i = 1; i <= 10; ++i){
    std::lock_guard<std::mutex> lock(mtx);
    cv.wait(lock, [] { return !ready;});
    
    shared_value = i;
    ready = true;
    cv.notify_one()
  }
  {
    std::lock_guard<std::mutex> lock(mtx);
    done = true;
    cv.notify_one();
  }
}

void consumer(Logger& logger){
  while (true){
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [] { return ready || done; });
    if (done && !ready){
      break;
    }
  int value = shared_value;
  ready = false;
  cv.notify_one();
  lock.unlock();
  
  std::ostringstream oss;
  oss << "[Consumer] got value: " << value << "\n";
  logger.writeLine(oss.str());
  }
}
