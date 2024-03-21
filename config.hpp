#pragma once

struct GameConfig  { /* constexpr indeed */
  constexpr static int num_robot=10, num_berth=10, num_boat=5;
  constexpr static int map_width=200, map_height=200;
  constexpr static int max_frame=15001;
  constexpr static int block_num=5;
};