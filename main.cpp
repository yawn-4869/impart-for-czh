#include "config.hpp"
#include "entity.hpp"
#include "reader.hpp"
#include "finder.hpp"

inline void stream_io_init() {
  std::ios::sync_with_stdio(false);
  // std::cin.tie(nullptr);
  // std::cout.tie(nullptr);
}

inline void go_next() {
  std::cout << "OK" << std::endl;
}

void template_main() {
  stream_io_init();
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
    reader.update_frame<std::deque, std::vector, std::vector>(game_status, goods, map.robots, boats);
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
  stream_io_init();
  GameConfig initial_config;
  EqWeightGrid map;
  Reader reader;
  reader.initilize(map, initial_config);

  GameStatus game_status;
  std::deque<Goods> goods;
  std::vector<Boat> boats(initial_config.num_boat, map.capacity);
  // forward GOODS
  std::vector<std::unordered_map<GridLocation, GridLocation>> robot_came_from(initial_config.num_robot);
  std::vector<std::unordered_map<GridLocation, double>> robot_cost_so_far(initial_config.num_robot);
  std::vector<int32_t> robot_task(initial_config.num_robot, -1);
  std::vector<std::vector<GridLocation>> to_goods(initial_config.num_robot); /* track from goods to robot */
  // forward BERTH
  std::vector<std::unordered_map<GridLocation, GridLocation>> berth_came_from(initial_config.num_robot);
  std::vector<std::unordered_map<GridLocation, double>> berth_cost_so_far(initial_config.num_robot);
  std::vector<int32_t> robot_terminal(initial_config.num_robot, -1);
  std::vector<std::vector<GridLocation>> to_berth(initial_config.num_robot); /* track from berth to goods */

  std::vector<int32_t> robot_forward(initial_config.num_robot); /* (track_idx)  */
  std::vector<int32_t> robot_backward(initial_config.num_robot);
  std::vector<int32_t*> robot_dir(initial_config.num_robot, nullptr);

  go_next();

  for (int32_t frame = 0; frame < initial_config.max_frame; ++frame) {
    // get new robot position from new frame (and it will order the robots)
    reader.update_frame<std::deque, std::vector, std::vector>(game_status, goods, map.robots, boats);
    
    // just use one robot for test
    if (robot_task[0] == -1) {
      robot_task[0] = target_select<Goods, std::deque>(map.robots[0], goods, [](Goods goods, int32_t pred)->int32_t{
        return goods.value - pred;
      }); // select one good
      search(map, map.robots[0].pos, goods[robot_task[0]].pos, robot_came_from[0], robot_cost_so_far[0]);
      trace<GridLocation>(robot_came_from[0], to_goods[0], goods[robot_task[0]].pos);
      robot_forward[0] = to_goods[0].size()-1;
      robot_dir[0] = &robot_forward[0];
    }
    // on the way
    if(*robot_dir[0] == 0) {
      if(map.robots[0].goods) {
        // already collect goods. on the way to berth
        std::cout << "pull " << 0 << std::endl;
        map.robots[0].goods = false;
        robot_task[0] = -1;
      } else {
        // on the way to goods
        std::cout << "get " << 0 << std::endl;
        map.robots[0].goods = true;
        robot_dir[0] = &robot_backward[0];

        robot_terminal[0] = target_select<Berth, std::vector>(map.robots[0], map.terminals, [](Berth b, int32_t pred)->int32_t{
          return b.load_speed*100 - pred;
        });
        search(map, goods[robot_task[0]].pos, map.terminals[robot_terminal[0]].pos, berth_came_from[0], berth_cost_so_far[0]);
        trace(berth_came_from[0], to_berth[0], map.terminals[robot_terminal[0]].pos);
        robot_backward[0] = to_berth[0].size()-1;
      }
    } else {
      GridLocation target = map.robots[0].goods? to_berth[0][*robot_dir[0]] : to_goods[0][*robot_dir[0]];
      std::cout << "move " << 0 << ' ' << SquareGrid::get_dirs_index(map.robots[0].pos, target) << std::endl;
      *robot_dir[0]--;
    }
    go_next();
  }
  go_next(); // game over
}