#pragma once
// reviewed: 2024-01-04
// reviewed: 2024-01-08
// reviewed: 2024-07-08
// reviewed: 2024-07-15

inline static auto rnd1(float const range) -> float {
  int const r = rand();
  if (debug_multiplayer) {
    printf("%lu: %lu: rnd1: %d\n", frame_context.frame_num, frame_context.ms,
           r);
  }
  return float(r) / float(RAND_MAX) * range - range / 2;
}

inline static auto rnd2(float const zero_to_range) -> float {
  int const r = rand();
  if (debug_multiplayer) {
    printf("%lu: %lu: rnd2: %d\n", frame_context.frame_num, frame_context.ms,
           r);
  }
  return float(r) / float(RAND_MAX) * zero_to_range;
}

inline static auto rnd3(int const rem) -> bool {
  int const r = rand();
  if (debug_multiplayer) {
    printf("%lu: %lu: rnd3: %d\n", frame_context.frame_num, frame_context.ms,
           r);
  }
  return (r % rem) == 0;
}

inline static auto game_area_roll(vec3 &position) -> void {
  // approximately correct. can be done better.

  if (position.x < game_area_min_x) {
    position.x = game_area_max_x;
  } else if (position.x > game_area_max_x) {
    position.x = game_area_min_x;
  }

  if (position.y < game_area_min_y) {
    position.y = game_area_max_y;
  } else if (position.y > game_area_max_y) {
    position.y = game_area_min_y;
  }

  if (position.z < game_area_min_z) {
    position.z = game_area_max_z;
  } else if (position.z > game_area_max_z) {
    position.z = game_area_min_z;
  }
}

inline static auto is_outside_game_area(vec3 const &position) -> bool {
  return position.x < game_area_min_x || position.x > game_area_max_x ||
         position.y < game_area_min_y || position.y > game_area_max_y ||
         position.z < game_area_min_z || position.z > game_area_max_z;
}
