# sokoban-solver-1

This solver can solve Sokoban puzzles if they are not simple enought (like the one included in the sample_level directory).

To buid, run

$ make

at the project root. If it fails, fixing it should be straightforward since the code uses ony the standard C++17.

To run the executable, run

$ ./sokoban-solver < sample_levels/sample_level1.txt

It will print to stdout complete steps from the start to finish.
