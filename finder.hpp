#include <functional>

#include "entity.hpp"

inline double manhattan(GridLocation a, GridLocation b) {
  return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

template <typename Location, template <typename> typename Sequence>
void select(const Robot& robot, Sequence<Goods>& goods){
  int32_t min_pred_time=40000, current=0;
  for (int32_t i=0; i<goods.size(); ++i) {
    current = manhattan(robot.pos, goods[i].pos);
    
  }
}

template <typename Location, typename Grid>
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
        came_from[next] = current;
      }
    }
  }
}