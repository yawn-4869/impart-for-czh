#include <functional>

#include "entity.hpp"
#include "logger.hpp"

inline double manhattan(GridLocation a, GridLocation b) {
  return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

template <typename El, template <typename> typename Sequence>
int32_t target_select(const Robot& robot, Sequence<El>& elements, std::function<int32_t(El, int32_t)> value_ordering) {
  int32_t max_value = -400, pred_dist = 0, cur_value, elem_idx;
  for (int32_t i = 0; i < elements.size(); ++i) {
    pred_dist = manhattan(robot.pos, elements[i].pos);
    cur_value = value_ordering(elements[i], pred_dist);// value ordering
    if (cur_value > max_value) {
      max_value = cur_value;
      elem_idx = i;
    }
  }
  return elem_idx;
}

template <typename Grid, typename Location>
void search /* indeed a star search*/
    (Grid& grid, Location launch, Location target,
     std::unordered_map<Location, Location>& came_from,
     std::unordered_map<Location, double>& cost_so_far,
     std::function<double(Location a, Location b)> heuristic = manhattan) {
  PriorityQueue<Location, double> frontier;
  frontier.put(launch, 0);

  came_from[launch] = launch;
  cost_so_far[launch] = 0;

  while (!frontier.empty()) {
    Location current = frontier.get();

    if (current == target) {
      break;
    }

    for (Location next : grid.neighbors(current)) {
      double new_cost = cost_so_far[current] + grid.cost(current, next);
      if (cost_so_far.find(next) == cost_so_far.end() ||
          new_cost < cost_so_far[next]) {
        cost_so_far[next] = new_cost;
        double priority = new_cost + heuristic(next, target);
        frontier.put(next, priority);
        came_from[next] = current; // lastly came_from[target] = target.previous
      }
    }
  }
}


template <typename Location>
void trace(std::unordered_map<Location, Location>& came_from,
           std::vector<Location>& track, Location target) {
            // check whether it is full
  track.emplace_back(target);
  Location previous = came_from[target];
  while (previous != came_from[previous]) {
    track.emplace_back(previous);
    previous = came_from[previous];
  }
  logger->log("trace", track); // target, target.previous, ..., start.next
}