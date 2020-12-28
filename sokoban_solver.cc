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

  // Initialize all member values.
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

  // The main loop.
  while (!q_.empty()) {
    const GDSInfo* gdsi = q_.top();
    q_.pop();
    
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
    
    // Add moves into neighboring spaces.
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

bool Solver::IsWall(Square square) const {
  return !IsElementOf(square, level_.floors);
}

constexpr int adjacent_deltas[4][2] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};

std::vector<Solver::GDS> Solver::GenerateNext(const Solver::GDS& gds) {
  std::vector<GDS> next_steps;

  const auto player = gds.player;
  const auto& boxes = gds.boxes;

  //  for (const auto [dx, dy] : {{0, 1}, {0, -1}, {1, 0}, {-1, 0}}) {
  for (const auto [dx, dy] : adjacent_deltas) {
    const Square adjacent(player.x() + dx, player.y() + dy);
    
    // If the adjacent square is a wall, the player cannot move there.
    if (IsWall(adjacent)) {
      continue;
    }

    // If the adjacent square is not occupied by a box, move the player into it.
    if (!IsElementOf(adjacent, boxes)) {
      next_steps.emplace_back(boxes, adjacent);
      continue;
    }

    // If the adjacent square is a box that is not blocked by a wall or other box,
    // let the player push it.
    const Square behind_adjacent(adjacent.x() + dx, adjacent.y() + dy);
    if (!IsWall(behind_adjacent) && !IsElementOf(behind_adjacent, boxes)) {
      SquareSet new_boxes(boxes);
      new_boxes.erase(adjacent);
      new_boxes.insert(behind_adjacent);
      next_steps.emplace_back(std::move(new_boxes), adjacent);
    }
  }

  return next_steps;
}

}  // namespace sokoban
