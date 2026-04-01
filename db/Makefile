CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
LDFLAGS = -lpqxx -lpq # linker errors otherwise, prob a missing function

# The goal is to make each c file into its own executable by default
SRCS = $(wildcard *.cpp)
EXES = $(SRCS:.cpp=) # Replace .c extension with space

all: $(EXES)

$(EXES): %: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(EXES) *.o
