#include "sokoban_solver.h"

#include <algorithm>
#include <cassert>
#include <iostream>

namespace sokoban {

namespace {

inline int ManhattanDistance(const Solver::Square sq1, const Solver::Square sq2) {
  return std::abs(sq1.x() - sq2.x()) + std::abs(sq1.y() - sq2.y());
}

inline bool IsElementOf(Solver::Square square, const Solver::SquareSet& square_set) {
  return square_set.find(square) != square_set.end();
}

} // namespace

std::vector<Level> Solver::Solve(const Level& level) {
  // Sanity check the level.
  assert(IsElementOf(level.player, level.floors));
  assert(level.boxes.size() == level.goals.size());
  assert(std::includes(std::begin(level.floors), std::end(level.floors),
                       std::begin(level.boxes), std::end(level.boxes)));
  assert(std::includes(std::begin(level.floors), std::end(level.floors),
                       std::begin(level.goals), std::end(level.goals)));

  // Initialize all member variables.
  level_ = level;
  gds_entries_.clear();
  gds_map_.clear();
  while (!q_.empty()) {
    q_.pop();
  }

  auto add_gds = [this](int predecessor_id, GDS&& gds) mutable {
    auto gds_info = std::make_unique<GDSInfo>(
        gds_entries_.size(), predecessor_id, CalcScore(gds), std::move(gds));
    gds_map_.emplace(&gds_info->gds, gds_info.get());
    q_.push(gds_info.get());
    gds_entries_.push_back(std::move(gds_info));
  };

  // Add the initial GDS.
  constexpr int kCentinelID = -1;
  add_gds(kCentinelID, GDS(level.boxes, level.player));

  uint64_t last_gds_count = 0;

  // The main loop.
  while (!q_.empty()) {
    const GDSInfo* gdsi = q_.top();
    q_.pop();

    if (gds_entries_.size() - last_gds_count >= 1000000) {
      std::cerr << "PROGRESS: gds_count=" << gds_entries_.size() << ", q_count=" << q_.size() << std::endl;
      last_gds_count = gds_entries_.size();
    }

    // Found a solution?
    if (gdsi->score == 0) {
      std::cerr << "=== Stats BEGIN ===" << '\n'
                << "gds_entries_.size()=" << gds_entries_.size() << '\n'
                << "q_.size()=" << q_.size() << '\n'
                << "=== Stats END ===" << '\n';

      std::vector<Level> solution;

      // Add all steps to the solution.
      for (int id = gdsi->id; id != kCentinelID; id = gds_entries_[id]->predecessor_id) {
        const auto& gds = gds_entries_[id]->gds;
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
      if (gds_map_.find(&next_gds) == gds_map_.end()) {
        add_gds(gdsi->id, std::move(next_gds));
      }
    }
  }

  return {};
}

int Solver::CalcScore(const Solver::GDS& gds) const {
  int score = 0;
  auto it = level_.goals.cbegin();
  for (const auto& square : gds.boxes) {
    score += ManhattanDistance(*it++, square);
  }
  return score;
}

bool Solver::IsGoal(Square square) const {
  return IsElementOf(square, level_.goals);
}

bool Solver::IsWall(Square square) const {
  return !IsElementOf(square, level_.floors);
}

bool Solver::IsOccupied(Square square, const SquareSet& boxes) const {
  return IsWall(square) || IsElementOf(square, boxes);
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
    if (!IsElementOf(adjacent, boxes)) {
      next_steps.emplace_back(boxes, adjacent);
      continue;
    }

    auto is_deadlocked = [this](const auto player, const auto& boxes,
                          const auto behind_adjacent, const auto& four_corner_deadend) {
      const auto sq1 = player + four_corner_deadend[0];
      const bool sq1_is_wall = IsWall(sq1);
      const bool sq1_is_box = IsElementOf(sq1, boxes);
      if (!sq1_is_wall && !sq1_is_box) { return false; }

      const auto sq2 = player + four_corner_deadend[1];
      const bool sq2_is_wall = IsWall(sq2);
      const bool sq2_is_box = IsElementOf(sq2, boxes);
      if (!sq2_is_wall && !sq2_is_box) { return false; }

      const auto sq3 = player + four_corner_deadend[2];
      const bool sq3_is_wall = IsWall(sq3);
      const bool sq3_is_box = IsElementOf(sq3, boxes);
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
        !is_deadlocked(player, boxes, behind_adjacent, square_check.four_corner_deadend1) &&
        !is_deadlocked(player, boxes, behind_adjacent, square_check.four_corner_deadend2)) {
      SquareSet new_boxes(boxes);
      new_boxes.erase(adjacent);
      new_boxes.insert(behind_adjacent);
      next_steps.emplace_back(std::move(new_boxes), adjacent);
    }
  }

  return next_steps;
}

}  // namespace sokoban
