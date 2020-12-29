#include <iostream>

#include "sokoban_level_reader_writer.h"
#include "sokoban_solver.h"

int main(int argc, char** argv) {
  std::istreambuf_iterator<char> begin(std::cin), end;
  const std::string level_string(begin, end);
  std::cerr << "Input:\n" << level_string << "\n";

  const auto level = sokoban::StringToLevel(level_string);
  std::cerr << "Solving:\n" << sokoban::LevelToString(level) << "\n";

  sokoban::Solver solver;
  const auto solution_steps = solver.Solve(level);
  
  std::cerr << "Final game dynamic states: " << solver.GetDynamicStateSize() << '\n'
            << "Final priority queue size: " << solver.GetQueueSize() << '\n';

  if (solution_steps.empty()) {
    std::cerr << "The level is not solvable." << std::endl;
    return -1;
  }

  std::cerr << "Found a solution!" << std::endl;
  int n = 1;
  for (const auto& step : solution_steps) {
    std::cout << "Step " << n++ << ": player=(" << step.player.x() << ", " << step.player.y() << ")\n"
              << sokoban::LevelToString(step) << '\n';
  }

  return 0;
}
