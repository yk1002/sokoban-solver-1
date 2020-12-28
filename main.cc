#include <iostream>

#include "sokoban_level_reader_writer.h"
#include "sokoban_solver.h"

int main(int argc, char** argv) {
  std::istreambuf_iterator<char> begin(std::cin), end;
  const std::string level_string(begin, end);
  std::cerr << "Input:\n" << level_string << "\n";

  const auto level = sokoban::StringToLevel(level_string);
  const auto solution_steps = sokoban::Solver().Solve(level);

  if (solution_steps.empty()) {
    std::cerr << "No solution." << std::endl;
    return -1;
  }

  int n = 1;
  for (const auto& step : solution_steps) {
    std::cout << "Step " << n++ << ": player=(" << step.player.x() << ", " << step.player.y() << ")\n"
              << sokoban::LevelToString(step) << '\n';
  }

  return 0;
}
