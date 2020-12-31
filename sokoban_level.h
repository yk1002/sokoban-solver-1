#pragma once

#include <algorithm>
#include <initializer_list>
#include <utility>
#include <vector>

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

  class SquareSet {
   public:
    using iterator = typename std::vector<Square>::iterator;
    using const_iterator = typename std::vector<Square>::const_iterator;

    SquareSet() = default;
    SquareSet(const SquareSet& orig) {
      values_.reserve(orig.values_.size());
      values_ = orig.values_;
    }
    SquareSet(std::initializer_list<Square> il) : values_(il) {
      std::sort(std::begin(values_), std::end(values_));
    }

    SquareSet& operator = (const SquareSet& rhs) {
      values_.reserve(rhs.values_.size());
      values_ = rhs.values_;
      return *this;
    }

    bool operator == (const SquareSet& rhs) const {
      return values_ == rhs.values_;
    }

    void Add(const Square square) {
      for (auto it = values_.begin(); it != values_.end(); ++it) {
        if (square == *it) {
          return;
        } else if (square < *it) {
          values_.insert(it, square);
          return;
        }
      }
      values_.emplace_back(square);
    }

    bool Contains(const Square square) const {
      for (const auto& elem : values_) {
        if (square == elem) {
          return true;
        }
        if (square < elem) {
          return false;
        }
      }
      return false;
    }

    void Replace(const Square replaced, const Square replacing) {
      const auto it = std::find(std::begin(values_), std::end(values_), replaced);
      assert(it != values_.end());
      *it = replacing;
      std::sort(std::begin(values_), std::end(values_));
    }

    void ShrinkToFit() {
      values_.shrink_to_fit();
    }

    iterator begin() {
      return std::begin(values_);
    }

    const_iterator begin() const {
      return std::begin(values_);
    }

    iterator end() {
      return std::end(values_);
    }

    const_iterator end() const {
      return std::end(values_);
    }

    void clear() {
      return values_.clear();
    }

    size_t size() const {
      return values_.size();
    }

   private:
    std::vector<Square> values_;
  };

  Square player;
  SquareSet boxes;
  SquareSet goals;
  SquareSet floors; // is a super set of (player|boxes|goals).
};

}  // namespace sokoban
