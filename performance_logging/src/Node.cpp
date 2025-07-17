#include "../include/Node.hpp"
#include "../include/Logger.hpp"
#include <chrono>
#include <thread>
#include <sstream>

Node::Node(int nodeId, int maxCPU, int maxMemory, int maxIO)
    : id(nodeId),
      maxCPU(maxCPU),
      maxMemory(maxMemory),
      maxIO(maxIO),
      usedCPU(0),
      usedMemory(0),
      usedIO(0),
      running(false) {}

Node::~Node() {
    stop();
}

void Node::start() {
    running = true;
    worker = std::thread(&Node::process, this);
}

void Node::stop() {
    running = false;
    cv.notify_all();
    if (worker.joinable())
        worker.join();
}

void Node::assignTask(const Task& task) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        taskQueue.push(task);
        usedCPU += task.requiredCPU;
        usedMemory += task.requiredMemory;
        usedIO += task.requiredIO;
    }
    cv.notify_one();
}

bool Node::isAvailable(int cpu, int memory, int io) const {
    return (usedCPU + cpu <= maxCPU) &&
           (usedMemory + memory <= maxMemory) &&
           (usedIO + io <= maxIO);
}

void Node::process() {
    while (running) {
        Task task(0, 0, 0, 0);
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            cv.wait(lock, [&]() { return !taskQueue.empty() || !running; });

            if (!running) break;

            task = taskQueue.front();
            taskQueue.pop();
        }

        // Log task start
        std::ostringstream ss;
        ss << "Node " << id << " started Task " << task.id
           << " (CPU: " << task.requiredCPU
           << "%, Mem: " << task.requiredMemory << "MB, IO: " << task.requiredIO << " MB/s)";
        Logger::getInstance().log(ss.str());

        std::this_thread::sleep_for(std::chrono::seconds(2)); // Simulated task duration

        {
            std::lock_guard<std::mutex> lock(queueMutex);
            usedCPU -= task.requiredCPU;
            usedMemory -= task.requiredMemory;
            usedIO -= task.requiredIO;
        }

        // Log task completion
        ss.str("");
        ss << "Node " << id << " completed Task " << task.id;
        Logger::getInstance().log(ss.str());
    }
}
