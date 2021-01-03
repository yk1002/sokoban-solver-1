#pragma once

#include <limits>
#include <memory>
#include <queue>
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
  
  struct __attribute__ ((packed)) GDS { // Game Dynamic State
    GDS() = default;
    GDS(const SquareSet& boxes_in, Square player_in) : boxes(boxes_in), player(player_in) {}
    GDS(SquareSet&& boxes_in, Square player_in) : boxes(std::move(boxes_in)), player(player_in) {}
    SquareSet boxes;
    Square player;
  };
  
  using GDSInfoID = uint32_t;
  static constexpr GDSInfoID kInvalidGDSInfoID = std::numeric_limits<GDSInfoID>::max();

  using Score = uint16_t;

  struct __attribute__ ((packed)) GDSInfo {
    GDSInfo() = default;
    GDSInfo(GDSInfoID predecessor_id_in, uint16_t score_in, GDS&& gds_in) :
        predecessor_id(predecessor_id_in), score(score_in), gds(std::move(gds_in)) {}
    const GDSInfoID predecessor_id = -1;
    Score score = std::numeric_limits<Score>::max();
    GDS gds;
  };
    
  std::string SanityCheckLevel(const Level& level) const;
  void Initialize(const Level& level);
  Score CalcScore(const GDS& gds) const;
  bool IsGoal(Square square) const;
  bool IsWall(Square square) const;
  bool IsOccupied(Square square, const SquareSet& boxes) const;
  bool IsDeadendFloor(Square square) const;
  std::vector<GDS> GenerateNext(const GDS& gds);
  SquareSet FindDeadendFloors(const Level& level);

  struct GDSInfoHash {
    GDSInfoHash(const std::vector<GDSInfo>& gds_entries) : gds_entries_(gds_entries) {}
    size_t operator()(GDSInfoID id) const {
      const auto& gds = gds_entries_[id].gds;
      auto val = [](auto square) { return square.x() | (square.y() << 8); };

      std::size_t seed = 0;
      for(const auto& x : gds.boxes) {
        seed ^= val(x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
      }
      seed ^= val(gds.player) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
      return seed;
    }

    const std::vector<GDSInfo>& gds_entries_;
  };

  struct GDSInfoEqual {
    GDSInfoEqual(const std::vector<GDSInfo>& gds_entries) : gds_entries_(gds_entries) {}
    bool operator()(GDSInfoID id_lhs, GDSInfoID id_rhs) const {
      const auto& lhs = gds_entries_[id_lhs].gds;
      const auto& rhs = gds_entries_[id_rhs].gds;
      return lhs.boxes == rhs.boxes && lhs.player == rhs.player;
    }

    const std::vector<GDSInfo>& gds_entries_;
  };

  struct GDSInfoGreater {
    GDSInfoGreater(const std::vector<GDSInfo>& gds_entries) : gds_entries_(gds_entries) {}
    bool operator()(GDSInfoID id_lhs, GDSInfoID id_rhs) const {
      const auto& lhs = gds_entries_[id_lhs];
      const auto& rhs = gds_entries_[id_rhs];
      return (lhs.score > rhs.score) || (lhs.score == rhs.score && id_lhs > id_rhs);
    }

    const std::vector<GDSInfo>& gds_entries_;
  };

  Level level_;
  SquareSet deadend_floors_;
  std::vector<GDSInfo> gds_entries_;
  std::unordered_set<GDSInfoID, GDSInfoHash, GDSInfoEqual> gds_set_{8192, GDSInfoHash{gds_entries_},
        GDSInfoEqual{gds_entries_}};
  std::priority_queue<GDSInfoID, std::vector<GDSInfoID>, GDSInfoGreater> q_{GDSInfoGreater{gds_entries_}};
};

}  // namespace sokoban
