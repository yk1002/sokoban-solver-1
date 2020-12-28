CC = clang
CXX = clang++
CXXFLAGS = -g -std=c++17 -O2
INCLUDES =
LDFLAGS =
LIBS =
OBJS = main.o sokoban_level_reader_writer.o sokoban_solver.o 
TARGET = sokoban_solver

$(TARGET): $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LDFLAGS) $(LIBS)

.cc.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(INCLUDES)

clean:
	rm -rf $(OBJS) $(TARGET)
