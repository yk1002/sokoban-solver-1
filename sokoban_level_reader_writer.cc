#include "sokoban_level_reader_writer.h"

#include <iostream>

// ::::::::::::::::::::::::::: Board ::::::::::::::::::::::::::
// :: Legend.................:      :.................Legend ::
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// :: Wall...................: #  # :...................Wall ::
// :: Pusher.................: p  @ :.................Pusher ::
// :: Pusher on goal square..: P  + :..Pusher on goal square ::
// :: Box....................: b  $ :....................Box ::
// :: Box on goal square.....: B  * :.....Box on goal square ::
// :: Goal square............: .  . :............Goal square ::
// :: Floor..................:      :..................Floor ::
// :: Floor..................: -  _ :..................Floor ::

namespace sokoban {

Level StringToLevel(std::string level_string) {
  Level level;
  int x = 0;
  int y = 0;
  for (const auto c : level_string) {
    const Level::Square square(x, y);
    switch (c) {
      case '#':
        ++x;
        break;
        
      case 'p':
      case '@':
        level.player = square;
        level.floors.emplace(square);
        ++x;
        break;

      case 'P':
      case '+':
        level.player = square;
        level.goals.emplace(square);
        level.floors.emplace(square);
        ++x;
        break;

      case 'b':
      case '$':
        level.boxes.emplace(square);
        level.floors.emplace(square);
        ++x;
        break;

      case 'B':
      case '*':
        level.boxes.emplace(square);
        level.goals.emplace(square);
        level.floors.emplace(square);
        ++x;
        break;

      case '.':
        level.goals.emplace(square);
        level.floors.emplace(square);
        ++x;
        break;

      case ' ':
      case '_':
      case '-':
        level.floors.emplace(square);
        ++x;
        break;

      case '\n':
      case '|':
        x = 0;
        ++y;
        break;

      default:
        std::cerr << "Invalid character at (" << x << ", " << y << "): '" << c << "'\n";
        ++x;
        break;
    }
  }

  return level;
}

std::string LevelToString(const Level& level) {
  int x_max = 0;
  int y_max = 0;
  for (auto& floor : level.floors) {
    x_max = std::max(x_max, floor.x());
    y_max = std::max(y_max, floor.y());
  }

  auto is_in = [](auto sq, const auto& square_set) {
    return square_set.count(sq) == 1;
  };

  std::string result;
  for (int y = 0; y <= y_max + 1; ++y) {
    for (int x = 0; x <= x_max + 1; ++x) {
      const Level::Square square{x, y};
      const bool is_player = (level.player == square);
      const bool is_goal = is_in(square, level.goals);
      const bool is_box = is_in(square, level.boxes);
      const bool is_floor = is_in(square, level.floors);

      if (is_player && is_goal) {
        result += '+';
      } else if (is_player && !is_goal) {
        result += '@';
      } else if (is_goal && is_box) {
        result += '*';
      } else if (is_goal && !is_box) {
        result += '.';
      } else if (!is_goal && is_box) {
        result += '$';
      } else if (is_floor) {
        result += ' ';
      } else {
        result += '#';
      }
    }
    result += '\n';
  }

  return result;
}

}  // namespace sokoban
