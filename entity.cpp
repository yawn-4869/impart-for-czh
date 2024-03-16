#include "entity.hpp"

std::vector<GridLocation> SquareGrid::neighbors(
    GridLocation id) const noexcept {
  std::vector<GridLocation> results;

  for (GridLocation dir : DIRS) {
    GridLocation next{id.x + dir.x, id.y + dir.y};
    if (in_bounds(next) && passable(next)) {
      results.emplace_back(next);
    }
  }

  // TODO: It does not affect efficiency 
  // if ((id.x + id.y) % 2 == 0) {
  //   // make the path straighter
  //   std::reverse(results.begin(), results.end());
  // }

  return results;
}

std::array<GridLocation, 4> SquareGrid::DIRS = {
    /* Right, Left, Up, Down */
    /* East, West, North, South */
    GridLocation{1, 0}, GridLocation{-1, 0}, GridLocation{0, -1},
    GridLocation{0, 1}};
