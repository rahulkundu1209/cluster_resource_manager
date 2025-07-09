#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>
#include <random>
#include <fstream>

using namespace std;

struct Node {
    int id;
    bool isAlive;
    float cpuUsage;
    float memoryUsage;

    Node(int nodeId) : id(nodeId), isAlive(true), cpuUsage(0), memoryUsage(0) {}
};

mutex mtx;
vector<Node> clusterNodes;

//  randomly up/down node health
void simulateHealth(Node& node) {
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis(0.0, 1.0);

    // failure chances
    node.isAlive = dis(gen) > 0.1;
    node.cpuUsage = dis(gen) * 100;
    node.memoryUsage = dis(gen) * 100;
}
void monitorHealth(int intervalSec) {
    ofstream logFile("health_log.txt", ios::app);

    while (true) {
        this_thread::sleep_for(chrono::seconds(intervalSec));

        mtx.lock();
        cout << "🩺 Health Check Started...\n";
        for (auto& node : clusterNodes) {
            simulateHealth(node);
            logFile << "Node " << node.id 
                    << " | Status: " << (node.isAlive ? "UP" : "DOWN")
                    << " | CPU: " << node.cpuUsage << "%"
                    << " | Memory: " << node.memoryUsage << "%\n";

            cout << "Node " << node.id << " is " 
                 << (node.isAlive ? "✅ UP" : "❌ DOWN") 
                 << " | CPU: " << node.cpuUsage << "% | MEM: " << node.memoryUsage << "%\n";

            // Failover hook
            if (!node.isAlive) {
                cout << "⚠️ Node " << node.id << " failed! Triggering failover...\n";
                // Add failover logic here (reassign tasks, etc.)
            }
        }
        cout << "✅ Health Check Complete.\n\n";
        mtx.unlock();
    }

    logFile.close();
}

int main() {
    const int nodeCount = 5;
    const int checkIntervalSec = 5;
    for (int i = 1; i <= nodeCount; ++i) {
        clusterNodes.push_back(Node(i));
    }
    thread healthThread(monitorHealth, checkIntervalSec);
    healthThread.join();  

    return 0;
}
