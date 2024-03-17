#include "config.hpp"
#include "entity.hpp"
#include "reader.hpp"
#include "writer.hpp"
#include "finder.hpp"

inline void stream_io_init() {
  std::ios::sync_with_stdio(false);
  // std::cin.tie(nullptr);
  // std::cout.tie(nullptr);
}

const char* DIR_NAME[] = { "right", "left", "up", "down" };

int template_main() {
// int main() {
  stream_io_init();
  GameConfig initial_config;
  EqWeightGrid map;
  Reader reader;
  Writer writer;
  reader.initilize(map, initial_config);

  GameStatus game_status;
  std::deque<Goods> goods;
  std::vector<Boat> boats(initial_config.num_boat, map.capacity);

  writer.ok();
  for (int32_t bid=0; bid < map.robots.size(); ++bid) {
    logger->log("robot", map.robots[bid]);
  }

  for (int32_t frame = 0; frame < initial_config.max_frame; ++frame) {
    // reader.update(map, frame);
    logger->log("frame", "update frame: " + std::to_string(frame));
    reader.update_frame<std::deque, std::vector, std::vector>(game_status, goods, map.robots, boats);
    for(int i = 0; i < initial_config.num_robot; i ++) {
      // printf("move %d %d\n", i, rand() % 4);
      writer.move(i, rand() % 3 + 1);
    writer.ok();
    /**
     * TODO:
     * 1. clear old goods
    */
    // TODO: pathfinder
      // search(map, map.robots[i], goods[0], );
    }
  }
  writer.ok();
  writer.flush(); // game over
  return 0;
}

int main() {
  stream_io_init();
  GameConfig initial_config;
  EqWeightGrid map;
  Reader reader;
  Writer writer;
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
  std::vector<std::vector<GridLocation>> to_berth(initial_config.num_robot); /* track from berth to goods */
  std::vector<int32_t> robot_terminal(initial_config.num_robot, -1);

  std::vector<int32_t> robot_forward(initial_config.num_robot); /* (track_idx)  */
  std::vector<int32_t> robot_backward(initial_config.num_robot);
  std::vector<int32_t*> robot_dir(initial_config.num_robot, nullptr);

  writer.ok();

  const int32_t test_robot_id = 0;
  for (int32_t frame = 0; frame < initial_config.max_frame; ++frame) {
    // get new robot position from new frame (and it will order the robots)
    reader.update_frame<std::deque, std::vector, std::vector>(game_status, goods, map.robots, boats);
    
    // just use one robot for test
    if (robot_task[test_robot_id] == -1) { // The robot has not been assigned a task yet.
      robot_task[test_robot_id] = target_select<Goods, std::deque>(map.robots[test_robot_id], goods, [](Goods goods, int32_t pred)->int32_t{
        return goods.value - pred;
      }); // select one good
      search(map, map.robots[test_robot_id].pos, goods[robot_task[test_robot_id]].pos, robot_came_from[test_robot_id], robot_cost_so_far[test_robot_id]);
      trace<GridLocation>(robot_came_from[test_robot_id], to_goods[test_robot_id], goods[robot_task[test_robot_id]].pos);
      robot_forward[test_robot_id] = to_goods[test_robot_id].size()-1;
      robot_dir[test_robot_id] = &robot_forward[test_robot_id];
      goods.erase(goods.begin()+robot_task[test_robot_id]); // remove the assigned goods
      logger->log("to_goods", "robot_forward: "+ std::to_string(robot_forward[test_robot_id]));
      logger->log("robot", "track index(robot->goods): "+ std::to_string(*robot_dir[test_robot_id]));
    }
    // on the way
    logger->log("pre/index", "goods carrying: " + std::to_string(map.robots[test_robot_id].goods) + ", dir_index: " + std::to_string(*robot_dir[test_robot_id]));
    if(*robot_dir[test_robot_id] == -1) {
      if(map.robots[test_robot_id].goods) {
        // already collect goods. on the way to berth
        logger->log("pull", "robot " + std::to_string(test_robot_id) + " pull goods: " + std::to_string(robot_task[test_robot_id]));
        writer.pull(test_robot_id);
        map.robots[test_robot_id].goods = false;
        robot_task[test_robot_id] = -1;
        // clear puller track
        berth_came_from[test_robot_id].clear();
        berth_cost_so_far[test_robot_id].clear();
        to_berth[test_robot_id].clear();
      } else {
        logger->log("get", "robot " + std::to_string(test_robot_id) + " get goods: " + std::to_string(robot_task[test_robot_id]));
        writer.get(test_robot_id);
        map.robots[test_robot_id].goods = true;
        robot_dir[test_robot_id] = &robot_backward[test_robot_id];
        // clear getter track
        robot_came_from[test_robot_id].clear();
        robot_cost_so_far[test_robot_id].clear();
        to_goods[test_robot_id].clear();

        robot_terminal[test_robot_id] = target_select<Berth, std::vector>(map.robots[test_robot_id], map.terminals, [](Berth b, int32_t pred)->int32_t{
          return b.load_speed*100 - pred;
        });
        search(map, map.robots[test_robot_id].pos, map.terminals[robot_terminal[test_robot_id]].pos, berth_came_from[test_robot_id], berth_cost_so_far[test_robot_id]);
        trace(berth_came_from[test_robot_id], to_berth[test_robot_id], map.terminals[robot_terminal[test_robot_id]].pos);
        robot_backward[test_robot_id] = to_berth[test_robot_id].size()-1;
        logger->log("robot", "track index(goods->berth): "+ std::to_string(*robot_dir[test_robot_id]) + ", goods:" + std::to_string(map.robots[test_robot_id].goods));
        logger->log("to_berth", "robot_backward: "+ std::to_string(*robot_dir[test_robot_id]));
      }
    } else {
      GridLocation target = map.robots[test_robot_id].goods? to_berth[test_robot_id][*robot_dir[test_robot_id]] : to_goods[test_robot_id][*robot_dir[test_robot_id]];
      logger->log("dir", "robot: " + map.robots[test_robot_id].pos.to_string() + " -> target: " + target.to_string());
      int32_t dir = SquareGrid::get_dirs_index(map.robots[test_robot_id].pos, target);
      // logger->log("move", {"right", "left", "up", "down"}[dir]);
      logger->log("move", std::to_string(dir) + "/" + DIR_NAME[dir]);
      writer.move(0, dir);
      *robot_dir[test_robot_id] -= 1;
    }
    logger->log("post/index", "goods carrying: " + std::to_string(map.robots[test_robot_id].goods) + ", dir_index: " + std::to_string(*robot_dir[test_robot_id]));
    writer.ok();
  }
  writer.ok(); // game over
  return 0;
}