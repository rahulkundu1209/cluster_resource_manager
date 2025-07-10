CXX = g++
CXXFLAGS = -std=c++17 -pthread -Iinclude

SRC = src/main.cpp src/Logger.cpp src/Node.cpp
OBJ = $(SRC:.cpp=.o)
OUT = cluster_manager

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT)

clean:
	rm -f $(OUT) logs/performance.log
