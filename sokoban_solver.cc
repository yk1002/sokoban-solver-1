#include "sokoban_solver.h"

#include <algorithm>
#include <cassert>
#include <iostream>

namespace sokoban {

namespace {

template <typename Square>
int ManhattanDistance(const Square sq1, const Square sq2) {
  return std::abs(sq1.x() - sq2.x()) + std::abs(sq1.y() - sq2.y());
}

} // namespace

std::vector<Level> Solver::Solve(const Level& level, bool preanalyze) {
  // Sanity-check the level.
  const auto error_msg = SanityCheckLevel(level);
  if (!error_msg.empty()) {
    std::cerr << "ERROR: " << error_msg << std::endl;
    assert(error_msg.empty());
  }
  
  // Initialize all data members;
  Initialize(level);

  // Pre-analyze the level.
  if (preanalyze) {
    deadend_floors_ = FindDeadendFloors(level);
    std::cerr << "Done preanalysis" << std::endl;
  }

  auto add_gds = [this](GDSInfoID predecessor_id, GDS gds) mutable {
    const auto gds_info_id = static_cast<GDSInfoID>(gds_entries_.size());
    gds_entries_.emplace_back(predecessor_id, 0, std::move(gds));
    if (gds_set_.find(gds_info_id) == gds_set_.end()) {
      auto& new_entry = gds_entries_.back();
      new_entry.score = CalcScore(new_entry.gds);
      gds_set_.emplace(gds_info_id);
      q_.push(gds_info_id);
    } else {
      gds_entries_.pop_back();
    }
  };

  // Add the initial GDS.
  add_gds(kInvalidGDSInfoID, GDS(level_.boxes, level_.player));

  // The main loop.
  int count = 0;
  uint16_t best_score = 1 << 14;
  while (!q_.empty()) {
    const GDSInfoID gds_info_id = q_.top();
    q_.pop();

    const GDSInfo* gdsi = &gds_entries_[gds_info_id];

    best_score = std::min(best_score, gdsi->score);

    if ((++count & 0xfffff) == 0xfffff) {
      std::cerr << "PROGRESS: gds_count=" << gds_entries_.size()
                << ", q_count=" << q_.size()
                << ", best_score=" << best_score
                << std::endl;
    }

    // Found a solution?
    if (gdsi->score == 0) {
      std::vector<Level> solution;

      // Add all steps to the solution.
      for (int id = gds_info_id; id != kInvalidGDSInfoID; id = gds_entries_[id].predecessor_id) {
        const auto& gds = gds_entries_[id].gds;
        Level step(level_);
        step.player = gds.player;
        step.boxes = gds.boxes;
        solution.push_back(step);
      }
      std::reverse(solution.begin(), solution.end());

      return solution;
    }

    // Add adjacent GDSs to the queue.
    for (auto& next_gds : GenerateNext(gdsi->gds)) {
      add_gds(gds_info_id, std::move(next_gds));
    }
  }

  return {};
}

std::string Solver::SanityCheckLevel(const Level& level) const {
  if (!level.floors.Contains(level.player)) {
    return "Player is not on any floor square.";
  }
  if (level.boxes.Contains(level.player)) {
    return "Player is on the same square as one of the boxes.";
  }
  if (level.boxes.size() != level.goals.size()) {
    return "The number of boxes and the number of goals do not match.";
  }
  if (!std::includes(std::begin(level.floors), std::end(level.floors),
                     std::begin(level.boxes), std::end(level.boxes))) {
    return "Not all boxes are on floor squares.";
  }
  if (!std::includes(std::begin(level.floors), std::end(level.floors),
                     std::begin(level.goals), std::end(level.goals))) {
    return "Not all goals are on floor squares.";
  }

  return {};
}

void Solver::Initialize(const Level& level) {
  while (!q_.empty()) {
    q_.pop();
  }
  gds_set_.clear();
  gds_entries_.clear();
  deadend_floors_.clear();
  level_ = level;
}

Solver::Score Solver::CalcScore(const Solver::GDS& gds) const {
  Score score = 0;
  auto it = level_.goals.begin();
  for (const auto& square : gds.boxes) {
    score += static_cast<Score>(ManhattanDistance(*it++, square));
  }
  return score;
}

bool Solver::IsGoal(Square square) const {
  return level_.goals.Contains(square);
}

bool Solver::IsWall(Square square) const {
  return !level_.floors.Contains(square);
}

bool Solver::IsOccupied(Square square, const SquareSet& boxes) const {
  return IsWall(square) || boxes.Contains(square);
}

bool Solver::IsDeadendFloor(Square square) const {
  return deadend_floors_.Contains(square);
}

const struct SquareCheck {
  using Offset = Level::Square;
  Offset adjacent;
  Offset behind_adjacent;
  Offset four_corner_deadend1[3];
  Offset four_corner_deadend2[3];
} square_check_list[4] = {
  { // Pushing to right
    {+1, 0},  // adjacent
    {+2, 0},  // behind_adjacent
    { {+3, 0}, {+2, +1}, {+3, +1} },  // four_corner_deadend1
    { {+3, 0}, {+2, -1}, {+3, -1} },  // four_corner_deadend1
  },
  { // Pushing to left
    {-1, 0},  // adjacent
    {-2, 0},  // behind_adjacent
    { {-3, 0}, {-2, +1}, {-3, +1} },  // four_corner_deadend1
    { {-3, 0}, {-2, -1}, {-3, -1} },  // four_corner_deadend1
  },
  { // Pushing up
    {0, -1},  // adjacent
    {0, -2},  // behind_adjacent
    { {0, -3}, {+1, -2}, {+1, -3} },  // four_corner_deadend1
    { {0, -3}, {-1, -2}, {-1, -3} },  // four_corner_deadend1
  },
  { // Pushing down
    {0, +1},  // adjacent
    {0, +2},  // behind_adjacent
    { {0, +3}, {+1, +2}, {+1, +3} },  // four_corner_deadend1
    { {0, +3}, {-1, +2}, {-1, +3} },  // four_corner_deadend1
  },
};

std::vector<Solver::GDS> Solver::GenerateNext(const Solver::GDS& gds) {
  std::vector<GDS> next_steps;

  const auto player = gds.player;
  const auto& boxes = gds.boxes;

  for (const auto& square_check : square_check_list) {
    const auto adjacent = player + square_check.adjacent;

    // If the adjacent square is a wall, the player cannot move there.
    if (IsWall(adjacent)) {
      continue;
    }

    // If the adjacent square is not occupied by a box, move the player into it.
    if (!boxes.Contains(adjacent)) {
      next_steps.emplace_back(boxes, adjacent);
      continue;
    }

    auto is_deadlocked = [this](const auto player, const auto& boxes,
                          const auto behind_adjacent, const auto& four_corner_deadend) {
      const auto sq1 = player + four_corner_deadend[0];
      const bool sq1_is_wall = IsWall(sq1);
      const bool sq1_is_box = boxes.Contains(sq1);
      if (!sq1_is_wall && !sq1_is_box) { return false; }

      const auto sq2 = player + four_corner_deadend[1];
      const bool sq2_is_wall = IsWall(sq2);
      const bool sq2_is_box = boxes.Contains(sq2);
      if (!sq2_is_wall && !sq2_is_box) { return false; }

      const auto sq3 = player + four_corner_deadend[2];
      const bool sq3_is_wall = IsWall(sq3);
      const bool sq3_is_box = boxes.Contains(sq3);
      if (!sq3_is_wall && !sq3_is_box) { return false; }

      // All four corners will be occupied if the box is pushed into behind_adjacent.
      // It is a deadlock if any one of them is a box that is not on a goal.
      if (!IsGoal(behind_adjacent)) { return true; }
      if (sq1_is_box && !IsGoal(sq1)) { return true; }
      if (sq2_is_box && !IsGoal(sq2)) { return true; }
      if (sq3_is_box && !IsGoal(sq3)) { return true; }

      return false;
    };

    // If the adjacent square is a box that is not blocked by a wall or other box behind it,
    // the player may be able to push it. However, let's make sure that doing so will not cause
    // an immediate deadlock.
    const auto behind_adjacent = player + square_check.behind_adjacent;
    if (!IsOccupied(behind_adjacent, boxes) &&
        !IsDeadendFloor(behind_adjacent) &&
        !is_deadlocked(player, boxes, behind_adjacent, square_check.four_corner_deadend1) &&
        !is_deadlocked(player, boxes, behind_adjacent, square_check.four_corner_deadend2)) {
      SquareSet new_boxes(boxes);
      new_boxes.Replace(adjacent, behind_adjacent);
      next_steps.emplace_back(std::move(new_boxes), adjacent);
    }
  }

  return next_steps;
}

Solver::SquareSet Solver::FindDeadendFloors(const Level& level) {
  SquareSet deadend_floors;

  // Put a box at each floor and check if it can reach any goal. If not, that floor is a deadend floor.
  for (const auto floor : level.floors) {
    // You cannot put a box where the player is.
    if (floor == level.player) {
      continue;
    }
    bool can_reach_goal = false;
    for (const auto goal : level.goals) {
      const Level one_goal_level{
        .player = level.player,
        .boxes = SquareSet{{floor}},
        .goals = SquareSet{{goal}},
        .floors = level.floors,
      };
      const auto solution = Solver().Solve(one_goal_level, false);
      if (!solution.empty()) {
        can_reach_goal = true;
        break;
      }
    }
    if (!can_reach_goal) {
      deadend_floors.Add(floor);
    }
  }

  return deadend_floors;
}

}  // namespace sokoban
