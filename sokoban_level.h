#pragma once

#include <set>
#include <utility>

namespace sokoban {

struct Level {
  struct Square : std::pair<int8_t, int8_t> {
    Square(int x = -1, int y = -1) :
        std::pair<int8_t, int8_t>(static_cast<int8_t>(x), static_cast<int8_t>(y)) {}
    int x() const { return this->first; }
    int y() const { return this->second; }
  };

  using SquareSet = std::set<Square>;

  Square player;
  SquareSet boxes;
  SquareSet goals;
  SquareSet floors; // is a super set of (player|boxes|goals).
};

}  // namespace sokoban
