#pragma once

#include <memory>
#include <queue>
#include <set>
#include <unordered_map>
#include <vector>

#include "sokoban_level.h"

namespace sokoban {

class Solver {
 public:
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

  // Solve the given level and returns a series of levels that represents a solusion.
  // Empty if no solution.
  std::vector<Level> Solve(const Level& level);

 private:
  int CalcScore(const GDS& gds) const;
  bool IsWall(Square square) const;
  std::vector<GDS> GenerateNext(const GDS& gds);

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

  Level level_;
  std::vector<std::unique_ptr<GDSInfo>> gds_entries_;
  std::unordered_map<GDS*, GDSInfo*, GDSHash, GDSEqual> gds_map_;
  std::priority_queue<GDSInfo*, std::vector<GDSInfo*>, GDSInfoGreater> q_;
};

}  // namespace sokoban
