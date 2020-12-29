#pragma once

#include <memory>
#include <queue>
#include <set>
#include <unordered_set>
#include <vector>

#include "sokoban_level.h"

namespace sokoban {

class Solver {
 public:
  // Solve the given Sokoban level and returns a series of levels that represents a solution.
  // Returns an empty vector if no solution was found.
  std::vector<Level> Solve(const Level& level, bool preanalyze = true);

  // Returns the number of game dynamic states.
  size_t GetDynamicStateSize() const { return gds_entries_.size(); }

  // Returns the number of game dynamic states in the priority queue.
  size_t GetQueueSize() const { return q_.size(); }

 private:
  using Square = Level::Square;
  using SquareSet = Level::SquareSet;
  
  struct GDS { // Game Dynamic State
    GDS() = default;
    GDS(const SquareSet& boxes_in, Square player_in) : boxes(boxes_in), player(player_in) {}
    GDS(SquareSet&& boxes_in, Square player_in) : boxes(std::move(boxes_in)), player(player_in) {}
    SquareSet boxes;
    Square player;
  };
  
  struct GDSHash {
    std::size_t operator()(const GDS* gds) const {
      auto val = [](auto square) { return square.x() | (square.y() << 8); };

      std::size_t seed = 0;
      for(const auto& x : gds->boxes) {
        seed ^= val(x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
      }
      seed ^= val(gds->player) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
      return seed;
    }
  };

  struct GDSEqual {
    bool operator()(const GDS* lhs, const GDS* rhs) const {
      return lhs->boxes == rhs->boxes && lhs->player == rhs->player;
    }
  };

  struct GDSInfo {
    GDSInfo() = default;
    GDSInfo(int id_in, int predecessor_id_in, int score_in, GDS&& gds_in) :
        id(id_in), predecessor_id(predecessor_id_in), score(score_in), gds(std::move(gds_in)) {}
    int id = -1;
    int predecessor_id = -1;
    int score = -1;
    GDS gds;
  };
    
  struct GDSInfoGreater {
    bool operator()(const GDSInfo* lhs, const GDSInfo* rhs) const {
      return (lhs->score > rhs->score) || (lhs->score == rhs->score && lhs->id > rhs->id);
    }
  };

  std::string SanityCheckLevel(const Level& level) const;
  void Initialize(const Level& level);
  int CalcScore(const GDS& gds) const;
  bool IsGoal(Square square) const;
  bool IsWall(Square square) const;
  bool IsOccupied(Square square, const SquareSet& boxes) const;
  bool IsDeadendFloor(Square square) const;
  std::vector<GDS> GenerateNext(const GDS& gds);
  SquareSet FindDeadendFloors(const Level& level);

  Level level_;
  SquareSet deadend_floors_;
  std::vector<std::unique_ptr<GDSInfo>> gds_entries_;
  std::unordered_set<GDS*, GDSHash, GDSEqual> gds_set_;
  std::priority_queue<GDSInfo*, std::vector<GDSInfo*>, GDSInfoGreater> q_;
};

}  // namespace sokoban
