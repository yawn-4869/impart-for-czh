#pragma once
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

#define LOG_ENABLE 1

class DummyLogger {
 public:
  DummyLogger(const char* log_, std::ios_base::openmode mode = std::ios::app) {}
  template <typename T, typename... Args>
  void log(const char*, const T&, const Args&...) {}
  template <typename T, typename... Args>
  void log(const T&, const Args&...) {}
};

class Logger {
  std::string logfile;
  std::ofstream handler;
  void log_time_header() {
    auto now = std::chrono::system_clock::now();
    auto now_local = std::chrono::system_clock::to_time_t(now);
    std::tm* now_tm = std::localtime(&now_local);
    handler << '[' << now_tm->tm_hour << ':' << now_tm->tm_min << ':'
            << now_tm->tm_sec << ']';
  }

  template <typename T, typename... Args>
  void _log(const T& msg, const Args&... args) {
    handler << msg;

    if constexpr (sizeof...(Args) > 0) {
      _log(args...);
    }
  }

 public:
  Logger(const char* log_, std::ios_base::openmode mode = std::ios::out)
      : logfile(log_), handler(log_, mode) {}
  ~Logger() { handler.close(); }

  template <typename T>
  void log(const char* tag, const T& msg) {
    log_time_header();
    handler << '[' << tag << ']' << msg << std::endl;
  }

  template <typename T, typename... Args>
  void log(const char* tag, const T& msg, const Args&... args) {
    log_time_header();
    handler << '[' << tag << ']' << msg;

    if constexpr (sizeof...(Args) > 0) {
      _log(args...);
    }
    handler << std::endl;
  }
};

#if LOG_ENABLE == 1
static std::unique_ptr<Logger> logger = std::unique_ptr<Logger>(new Logger("cc.log"));
#else
static std::unique_ptr<DummyLogger> logger =
    std::make_unique<DummyLogger>("cc.log");
#endif