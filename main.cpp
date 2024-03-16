#include "config.hpp"
#include "entity.hpp"
#include "reader.hpp"
#include "finder.hpp"

void stream_io_init() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  std::cout.tie(nullptr);
}

void template_main() {
  GameConfig initial_config;
  EqWeightGrid map;
  Reader reader;

  GameStatus game_status;
  std::deque<Goods> goods;
  std::vector<Boat> boats(initial_config.num_boat);

  reader.initilize(map, initial_config);
  std::cout<< "OK" <<std::endl;

  for (int32_t frame = 0; frame < initial_config.max_frame; ++frame) {
    // reader.update(map, frame);
    reader.update_frame(game_status, goods, map.robots, boats);
    for(int i = 0; i < initial_config.num_robot; i ++) {
      printf("move %d %d\n", i, rand() % 4);
    /**
     * TODO:
     * 1. clear old goods
    */
    // TODO: pathfinder
      // search(map, map.robots[i], goods[0], );
    }
  }
  std::cout<< "OK" <<std::endl; // game over
}

int main() {
  GameConfig initial_config;
  EqWeightGrid map;
  Reader reader;

  GameStatus game_status;
  std::deque<Goods> goods;
  std::vector<Boat> boats(initial_config.num_boat);
  std::vector<std::unordered_map<GridLocation, GridLocation>> robot_came_from(initial_config.num_robot);
  std::vector<std::unordered_map<GridLocation, double>> robot_cost_so_far(initial_config.num_robot);
  std::vector<int32_t> robot_task(initial_config.num_robot);

  reader.initilize(map, initial_config);
  std::cout<< "OK" <<std::endl;

  for (int32_t frame = 0; frame < initial_config.max_frame; ++frame) {
    // get new robot position from new frame (and it will order the robots)
    reader.update_frame(game_status, goods, map.robots, boats);
    
    // just use one robot for test
    // search(map, )
  }
  std::cout<< "OK" <<std::endl; // game over
}