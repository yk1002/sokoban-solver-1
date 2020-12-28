#pragma once

#include <set>
#include <utility>

namespace sokoban {

struct Level {
  using CoordType = int8_t;
  using SquareBase = std::pair<CoordType, CoordType>;
  
  struct Square : SquareBase {
    Square() : Square(-1, -1) {}

    template <typename T1, typename T2>
    Square(T1 x, T2 y) : SquareBase(static_cast<CoordType>(x), static_cast<CoordType>(y)) {}

    int x() const { return first; }
    int y() const { return second; }

    Square operator+(Square rhs) const {
      return {first + rhs.first, second + rhs.second};
    }
  };

  using SquareSet = std::set<Square>;

  Square player;
  SquareSet boxes;
  SquareSet goals;
  SquareSet floors; // is a super set of (player|boxes|goals).
};

}  // namespace sokoban
