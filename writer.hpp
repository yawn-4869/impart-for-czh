#pragma once

#include <iostream>

struct Writer {
  inline void ok() { A0Out("OK"); }
  inline void pull(int32_t idx) { A1Out("pull", idx); }
  inline void get(int32_t idx) { A1Out("get", idx); }
  inline void move(int32_t idx, int32_t toward) { A2Out("move", idx, toward); }
  inline void go(int32_t idx) { A1Out("go", idx); }
  inline void ship(int32_t boat_idx, int32_t berth_idx) {
    A2Out("ship", boat_idx, berth_idx);
  }
  inline void flush() { std::cout << std::flush; }

 private:
  inline void A0Out(const char* msg) { std::cout << msg << '\n'; }
  inline void A1Out(const char* msg, int32_t a1) {
    std::cout << msg << ' ' << a1 << '\n';
  }
  inline void A2Out(const char* msg, int32_t a1, int32_t a2) {
    std::cout << msg << ' ' << a1 << ' ' << a2 << '\n';
  }
};