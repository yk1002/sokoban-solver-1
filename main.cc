#include <iostream>
#include <random>

#include "sokoban_level_reader_writer.h"
#include "sokoban_solver.h"

using Level = sokoban::Level;
using Solver = sokoban::Solver;
using Square = sokoban::Solver::Square;
using SquareSet = sokoban::Solver::SquareSet;

int main(int argc, char** argv) {
  std::istreambuf_iterator<char> begin(std::cin), end;
  const std::string level_string(begin, end);
  std::cerr << "Input:\n" << level_string << "\n";

  const auto level = sokoban::StringToLevel(level_string);
  // const auto output_level_string = sokoban::LevelToString(level);
  // std::cerr << "Interpreted As:\n" << output_level_string;

  const auto gdss = Solver().Solve(level);

  if (gdss.empty()) {
    std::cerr << "No solution." << std::endl;
    return -1;
  }

  int step = 1;
  for (const auto& gds : gdss) {
    std::cout << "Step " << step++ << ":";
    std::cout << " player=(" << gds.player.x() << ", " << gds.player.y() << ")\n";

    Level snapshot(level);
    snapshot.player = gds.player;
    snapshot.boxes = gds.boxes;
    std::cout << sokoban::LevelToString(snapshot) << '\n';
  }

  return 0;
}
