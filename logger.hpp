#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <memory>

#define LOG_ENABLE 1

class BaseLogger {
  public:
  virtual int32_t log(const char* tag, const char* msg) = 0;
  template <typename _T, template<typename> typename Sequence>
  int32_t log(const char* tag, Sequence<_T>& seq);
};

class DummyLogger: public BaseLogger {
  public:
  DummyLogger(const char* log_, std::ios_base::openmode mode = std::ios::app) {}
  int32_t log(const char* tag, const char* msg) {
    return 0;
  }
  template <typename _T, template<typename> typename Sequence>
  int32_t log(const char* tag, Sequence<_T>& seq) {return 0;}
};

class Logger: public BaseLogger {
  std::string logfile;
  std::ofstream handler;
  public:
  Logger(const char* log_, std::ios_base::openmode mode = std::ios::app): logfile(log_), handler(log_, mode) {}
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
  template <typename _T, template<typename> typename Sequence>
  int32_t log(const char* tag, Sequence<_T>& seq) {
    std::stringstream ss;
    for(const auto& e: seq) {
      ss << e << ", ";
    }
    return log(tag, ss.str().c_str());
  }

  // template <typename _T>
  // // int32_t log(const char* tag, const std::vector<_T>& seq) {
  // int32_t log(const char* tag, std::vector<_T>& seq) {
  //   std::stringstream ss;
  //   for(const auto& e: seq) {
  //     ss << e << ", ";
  //   }
  //   log(tag, ss.str().c_str());
  //   return ss.str().length();
  // }
};

#if LOG_ENABLE == 1
static std::unique_ptr<BaseLogger> logger = std::make_unique<Logger>("cc.log");
#else
static std::unique_ptr<BaseLogger> logger = std::make_unique<DummyLogger>("cc.log");
#endif