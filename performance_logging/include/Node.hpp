#ifndef NODE_HPP
#define NODE_HPP

#include "Task.hpp"
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>

class Node {
public:
    Node(int nodeId, int maxCPU, int maxMemory, int mayIO);
    ~Node();
    
    void start();
    void stop();
    void assignTask(const Task& task);
    bool isAvailable(int cpu, int memory, int IO) const;

private:
    void process();

    int id;
    int maxCPU;
    int maxMemory;
    int maxIO;

    int usedCPU;
    int usedMemory;
    int usedIO;

    std::thread worker;
    std::queue<Task> taskQueue;
    mutable std::mutex queueMutex;
    std::condition_variable cv;
    std::atomic<bool> running;
};

#endif
