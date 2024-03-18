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
  std::deque<Goods> goods; // FIXME: use set instead of deque
  // std::unordered_set<GridLocation> goods;

  std::vector<Boat> boats(initial_config.num_boat, map.capacity);
  #pragma region variables for robots
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
  std::vector<std::vector<GridLocation>*> robot_target_ptr(initial_config.num_robot, nullptr);
  #pragma endregion

  writer.ok();

  const int32_t test_robot_id = 0;
  for (int32_t frame = 0; frame < initial_config.max_frame; ++frame) {
    // get new robot position from new frame (and it will order the robots)
    reader.update_frame<std::deque, std::vector, std::vector>(game_status, goods, map.robots, boats);
    
    // just use one robot for test
    #pragma region robot pathfinder
    if (robot_task[test_robot_id] == -1) { // The robot has not been assigned a task yet.
      robot_task[test_robot_id] = target_select<Goods, std::deque>(map.robots[test_robot_id], goods, [](Goods goods, int32_t pred)->int32_t{
        return goods.value - pred;
      }); // select one good
      if (robot_task[test_robot_id] == -1) break; // no goods here
      search(map, map.robots[test_robot_id].pos, goods[robot_task[test_robot_id]].pos, robot_came_from[test_robot_id], robot_cost_so_far[test_robot_id]);
      trace<GridLocation>(robot_came_from[test_robot_id], to_goods[test_robot_id], goods[robot_task[test_robot_id]].pos);
      robot_forward[test_robot_id] = to_goods[test_robot_id].size()-1;
      robot_dir[test_robot_id] = &robot_forward[test_robot_id];
      goods.erase(goods.begin()+robot_task[test_robot_id]); // remove the assigned goods
      robot_target_ptr[test_robot_id] = &to_goods[test_robot_id];
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
        // the berth receives one goods
        map.terminals[robot_terminal[test_robot_id]].receive();
        logger->log("berth/load", "berth: " + std::to_string(robot_terminal[test_robot_id]) + " receives a goods, current goods: " + std::to_string(map.terminals[robot_terminal[test_robot_id]].goods_todo));
        robot_terminal[test_robot_id] = -1;
        // clear puller track
        berth_came_from[test_robot_id].clear();
        berth_cost_so_far[test_robot_id].clear();
        to_berth[test_robot_id].clear();
        robot_target_ptr[test_robot_id] = nullptr;
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
        robot_target_ptr[test_robot_id] = &to_berth[test_robot_id];
        logger->log("robot", "track index(goods->berth): "+ std::to_string(*robot_dir[test_robot_id]) + ", goods:" + std::to_string(map.robots[test_robot_id].goods));
        logger->log("to_berth", "robot_backward: "+ std::to_string(*robot_dir[test_robot_id]));
      }
    } else {
      GridLocation target = (*robot_target_ptr[test_robot_id])[*robot_dir[test_robot_id]];
      // GridLocation target = map.robots[test_robot_id].goods? to_berth[test_robot_id][*robot_dir[test_robot_id]] : to_goods[test_robot_id][*robot_dir[test_robot_id]];
      logger->log("dir", "robot: " + map.robots[test_robot_id].pos.to_string() + " -> target: " + target.to_string());
      try {
        int32_t dir = SquareGrid::get_dirs_index(map.robots[test_robot_id].pos, target);
        // logger->log("move", {"right", "left", "up", "down"}[dir]);
        logger->log("move", std::to_string(dir) + "/" + DIR_NAME[dir]);
        writer.move(0, dir);
        *robot_dir[test_robot_id] -= 1;
      } catch (const std::runtime_error& e) { // runtime_error for no dir found
        logger->log("move/dir", std::string("dir error: ") + e.what() + "\n\ttarget_ptr: " + std::to_string((int64_t)robot_target_ptr[test_robot_id]) + "\n\tto_berth.addr: " + std::to_string((int64_t)&to_berth[test_robot_id]) + "\n\tto_goods.addr: " + std::to_string((int64_t)&to_goods[test_robot_id]) + "\nrobot_task: " + std::to_string(robot_task[test_robot_id]));
      }
    }
    logger->log("post/index", "goods carrying: " + std::to_string(map.robots[test_robot_id].goods) + ", dir_index: " + std::to_string(*robot_dir[test_robot_id]) + ", target berth: " + std::to_string(robot_terminal[test_robot_id]));
    #pragma endregion
    #pragma region ship/berth load and tranport
    // update dock info for berths
    for (int32_t i=0; i<boats.size(); ++i) {
      if (boats[i].dock != -1 && boats[i].status == 1 && map.terminals[boats[i].dock].dock_boat_id == -1) {
        bool dock_result = map.terminals[boats[i].dock].dock(i);
        if (dock_result) {
          logger->log("dock", "boat: " + std::to_string(i) + " docked at berth: " + std::to_string(boats[i].dock));
        }
      }
      else if (boats[i].status == 2) {
        // move the waiting boat to other berth
        for (int32_t j=0; j<map.terminals.size(); ++j) {
          if (map.terminals[j].goods_todo > 0 && map.terminals[j].dock_boat_id == -1) {
            boats[i].dockit(j);
            writer.ship(i, j);
            logger->log("reship", "boat: " + std::to_string(i) + " goes to berth: " + std::to_string(j));
            break;
          }
        }
      }
    }
    // trash logic for earn gold: all the boats serve the test one
    int32_t nearest_berth = robot_terminal[test_robot_id]; // desired berth
    // send a boat here
    if (nearest_berth != -1) {
      for (int32_t i=0; i<boats.size(); ++i) { // a shipping map should be here
        if (boats[i].dock == -1 && boats[i].status == 1) {
          boats[i].dockit(nearest_berth);
          writer.ship(i, nearest_berth);
          logger->log("ship/near", "boat: " + std::to_string(i) + " goes to berth: " + std::to_string(nearest_berth));
          break;
        }
      }
    }
    // if (nearest_berth != -1) {
    //   if (map.terminals[nearest_berth].dock_boat_id != -1) {
    //     // calculate remaining goods
    //     // whether here is any goods
    //     Berth& worker = map.terminals[nearest_berth];
    //     worker.load();
    //     logger->log("berth/load", "berth: " + std::to_string(nearest_berth) + " goods remaining: " + std::to_string(worker.goods_todo) + "goods have been loaded: " + std::to_string(worker.goods_done));
    //     int32_t boat_id = worker.dock_boat_id;
    //     bool get_goods = worker.goods_done > 0;
    //     bool fully_loaded = boats[boat_id].capacity == worker.goods_done;
    //     bool goods_clear = worker.goods_todo == 0;
    //     if (get_goods && (goods_clear || fully_loaded)) { // the boat has loaded some goods (or fully loaded) and there is no remaining goods
    //       boats[boat_id].leave();
    //       worker.leave();
    //       writer.go(boat_id);
    //       logger->log("go", "boat: " + std::to_string(boat_id) + " leaves from berth: " + std::to_string(nearest_berth));
    //     }
    //   }
    //   for (int32_t i=0; i<boats.size(); ++i) {
    //     if (boats[i].dock == -1 && boats[i].status == 1) {
    //       boats[i].dockit(nearest_berth);
    //       writer.ship(i, nearest_berth);
    //       logger->log("ship", "boat: " + std::to_string(i) + " goes to berth: " + std::to_string(nearest_berth));
    //     }
    //   }
    // }
    // iterate over all berths and send the boats
    for (int32_t i=0; i<map.terminals.size(); ++i) {
      int32_t nearest_berth = i; // reuse the old code
      if (map.terminals[nearest_berth].dock_boat_id != -1) {
        // calculate remaining goods
        // whether here is any goods
        Berth& worker = map.terminals[nearest_berth];
        logger->log("berth/info", "berth: " + std::to_string(nearest_berth) + " goods remaining: " + std::to_string(worker.goods_todo) + ", goods have been loaded: " + std::to_string(worker.goods_done));
        bool finish = worker.load();
        int32_t boat_id = worker.dock_boat_id;
        bool get_goods = worker.goods_done > 0;
        bool fully_loaded = boats[boat_id].capacity == worker.goods_done;
        bool goods_clear = worker.goods_todo == 0;
        logger->log("leave/check", "boat: " + std::to_string(boat_id) + ", goods loaded: " + std::to_string(worker.goods_done) + ", fully_loaded: " + std::to_string(fully_loaded));
        if (get_goods && (goods_clear || fully_loaded)) { // the boat has loaded some goods (or fully loaded) and there is no remaining goods
          logger->log("go", "boat: " + std::to_string(boat_id) + " leaves from berth: " + std::to_string(nearest_berth) + " with goods carrying: " + std::to_string(worker.goods_done) + " it should score at frame: " + std::to_string(frame + worker.transport_time));
          boats[boat_id].leave();
          worker.leave();
          writer.go(boat_id);
        }
      }
      // WTF logic
      for (int32_t i=0; i<boats.size(); ++i) {
        if (boats[i].dock == -1 && boats[i].status == 1 && ! boats[i].leaving) {
          boats[i].dockit(nearest_berth);
          writer.ship(i, nearest_berth);
          logger->log("ship/it", "boat: " + std::to_string(i) + " goes to berth: " + std::to_string(nearest_berth));
        }
      }
    }
    #pragma endregion
    writer.ok();
  }
  writer.ok(); // game over
  return 0;
}