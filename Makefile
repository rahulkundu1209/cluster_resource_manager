# Makefile for Cluster Resource Manager

CXX = g++
CXXFLAGS = -std=c++11 -pthread
TARGET = cluster_manager
SRC = main.cpp

all:
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET) performance.log
