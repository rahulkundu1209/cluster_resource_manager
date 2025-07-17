#include "../include/Task.hpp"
#include "../include/Node.hpp"
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

int main() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    const int numNodes = 3;
    const int numTasks = 10;

    std::vector<Node*> nodes;

    // Create nodes with max CPU, memory, and I/O
    for (int i = 0; i < numNodes; ++i) {
        nodes.push_back(new Node(i + 1, 100, 1024, 500)); // CPU:100%, Mem:1024MB, IO:500 MB/s
        nodes[i]->start();
    }

    // Generate and assign tasks
    for (int i = 0; i < numTasks; ++i) {
        int cpu = 10 + rand() % 30;     // 10% to 39%
        int mem = 100 + rand() % 300;   // 100MB to 399MB
        int io = 10 + rand() % 100;     // 10 to 109 MB/s
        Task t(i + 1, cpu, mem, io);

        bool assigned = false;
        for (auto& node : nodes) {
            if (node->isAvailable(cpu, mem, io)) {
                node->assignTask(t);
                assigned = true;
                break;
            }
        }

        if (!assigned) {
            std::cout << "Task " << t.id << " couldn't be assigned (Resource constraint).\n";
        }
    }

    // Let tasks run for some time
    std::this_thread::sleep_for(std::chrono::seconds(20));

    // Stop all nodes
    for (auto& node : nodes) {
        node->stop();
        delete node;
    }

    std::cout << "All nodes stopped.\n";
    return 0;
}
