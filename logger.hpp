#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <memory>
#include <type_traits>

#define LOG_ENABLE 1

class DummyLogger{
  public:
  DummyLogger(const char* log_, std::ios_base::openmode mode = std::ios::app) {}
  int32_t log(const char* tag, const char* msg) { return 0; }
  int32_t log(const char* tag, const std::string& msg) { return 0; }
  template <typename _T, template<typename> typename Sequence>
  int32_t log(const char* tag, Sequence<_T>& seq) {return 0;}
  template <typename _T>
  int32_t log(const char* tag, _T&) {return 0;}
};

class Logger{
  std::string logfile;
  std::ofstream handler;
  public:
  Logger(const char* log_, std::ios_base::openmode mode = std::ios::out): logfile(log_), handler(log_, mode) {}
  ~Logger() {
    handler.close();
  }
  int32_t log(const char* tag, const char* msg) {
    auto now = std::chrono::system_clock::now();
    auto now_local = std::chrono::system_clock::to_time_t(now);
    std::tm* now_tm = std::localtime(&now_local);
    std::stringstream ss;
    ss << '[' << now_tm->tm_hour << ':' << now_tm->tm_min << ':' << now_tm->tm_sec << ']';
    ss << '[' << tag << ']' << msg;
    handler << ss.str() << std::endl;
    return ss.str().length();
  }
  int32_t log(const char* tag, const std::string& msg) {
    return log(tag, msg.c_str());
  }
  template <typename _T, template<typename> typename Sequence>
  int32_t log(const char* tag, Sequence<_T>& seq) {
    std::stringstream ss;
    for(const auto& e: seq) {
      ss << e << ", ";
    }
    return log(tag, ss.str().c_str());
  }
  template <typename _T>
  int32_t log(const char* tag, _T& element) {
    const std::string& str = element.to_string();
    log(tag, str.c_str());
    return str.length();
  }

  // TODO: string concatenation delay optimization
  // template<typename T, typename... Args>
  // void log(T& first, Args&... args) {
  //   std::stringstream ss;

  //     if constexpr (std::is_same_v<T, int>) {
  //         std::cout << "int: " << first << std::endl;
  //     } else if constexpr (std::is_same_v<T, int*>) {
  //         std::cout << "int*: " << *first << std::endl;
  //     } else if constexpr (std::is_same_v<T, double>) {
  //         std::cout << "double: " << first << std::endl;
  //     }
  //     // 递归调用printArgs，展开剩余参数
  //     if constexpr (sizeof...(Args) > 0) {
  //         log(args...);
  //     }
  // }
};

#if LOG_ENABLE == 1
static std::unique_ptr<Logger> logger = std::make_unique<Logger>("cc.log");
#else
static std::unique_ptr<DummyLogger> logger = std::make_unique<DummyLogger>("cc.log");
#endif