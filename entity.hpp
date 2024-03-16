#pragma once

#include <algorithm>
#include <array>
#include <deque>
#include <iomanip>
#include <iostream>
#include <queue>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

struct GridLocation {
  int32_t x, y;
};


#pragma region GridLocation operators
static inline bool operator==(GridLocation lhs, GridLocation rhs) {
  return lhs.x == rhs.x && lhs.y == rhs.y;
}

static inline bool operator!=(GridLocation lhs, GridLocation rhs) {
  return !(lhs == rhs);
}

static inline bool operator<(GridLocation lhs, GridLocation rhs) {
  return std::tie(lhs.x, lhs.y) <
         std::tie(rhs.x, rhs.y);  // just a definition for sorting
}

static std::basic_iostream<char>::basic_ostream& operator<<(
    std::basic_iostream<char>::basic_ostream& out, GridLocation loc) {
  out << '(' << loc.x << ',' << loc.y << ')';
  return out;
}
#pragma endregion


namespace std {
/* implement hash function so we can put GridLocation into an unordered_set */
template <>
struct hash<GridLocation> {
  std::size_t operator()(const GridLocation& id) const noexcept {
    // NOTE: better to use something like boost hash_combine
    return std::hash<int32_t>()(id.x ^ (id.y << 16));
  }
};
}  // namespace std

#pragma region basic game entity

struct Robot {
  GridLocation pos;
  int32_t stay_frame; /* for resume reference */
  constexpr static int32_t STAY = 20; /* frames */
  bool running = true;
  bool goods = false;

  Robot() = default;
  Robot(int32_t x, int32_t y) : pos{x, y} {}
  Robot(GridLocation pos_) : pos(pos_) {}
};

struct Berth {
  constexpr static GridLocation size{4, 4};
  GridLocation pos;
  int32_t transport_time, load_time;

  Berth() = default;
  Berth(int32_t x, int32_t y, int32_t trans_time, int32_t load_time)
      : pos{x, y}, transport_time(trans_time), load_time(load_time) {}
};
constexpr int32_t transport_time(const Berth& from, const Berth& to) {
  return 500; /* frames */
}

struct Boat {
  constexpr static GridLocation size{2, 4};
  int32_t capacity, status /* 0: move, 1: load, 2: wait */, dock=-1;

  Boat() = default;
  Boat(int32_t capacity_) : capacity(capacity_) {}
};

struct GameStatus {
  int32_t frame, gold;
};

struct Goods {
  GridLocation pos;
  int32_t value, birthday, lifetime=1000 /* frames */;

  Goods(int32_t x, int32_t y, int32_t value_, int32_t birthday_): pos{x, y}, value(value_), birthday(birthday_) {}
};
#pragma endregion

struct SquareGrid {
  static std::array<GridLocation, 4> DIRS;

  int32_t width, height;
  std::unordered_set<GridLocation> walls;

  SquareGrid(int32_t width_, int32_t height_) : width(width_), height(height_) {}

  inline bool in_bounds(GridLocation id) const noexcept {
    return 0 <= id.x && id.x < width && 0 <= id.y && id.y < height;
  }

  inline bool passable(GridLocation id) const noexcept {
    return walls.find(id) == walls.end();
  }

  std::vector<GridLocation> neighbors(GridLocation id) const noexcept;
};



template <typename Grid>
void parse_surface_from_line(Grid& grid ,const std::string& line, int lineno) {
  for (int32_t i = 0; i < line.size(); ++i) {
    switch (line[i]) {
      case '*':
      case '#': 
        grid.walls.insert(GridLocation{lineno, i});
        break;
      case 'A':
        grid.robots.emplace_back(Robot(lineno, i));
        break;
    }
  }
}


struct EqWeightGrid : SquareGrid {
  std::vector<Berth> terminals; // terminal berths
  std::vector<Robot> robots;
  int32_t capacity;
  EqWeightGrid(int32_t w=200, int32_t h=200, int32_t num_bot=10, int32_t num_bth=10) : SquareGrid(w, h) { 
    robots.reserve(num_bot);
    terminals.reserve(num_bth);
  }
  constexpr double cost(const GridLocation& from_node,
                        const GridLocation& to_node) const noexcept {
    return 1;
  }
};

template <typename T, typename priority_t>
struct PriorityQueue {
  typedef std::pair<priority_t, T> PQElement;
  std::priority_queue<PQElement, std::vector<PQElement>,
                      std::greater<PQElement>>
      elements;  // min-heap

  inline bool empty() const { return elements.empty(); }

  inline void put(T item, priority_t priority) {
    elements.emplace(priority, item);
  }

  T get() {
    T best_item = elements.top().second;
    elements.pop();
    return best_item;
  }
};

struct Game {
  EqWeightGrid map;
  std::deque<Goods> goods;
  GameStatus status;

  Game() = default;
  void test(){
    // goods.erase
  }
};

