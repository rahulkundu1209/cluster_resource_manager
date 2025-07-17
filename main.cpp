#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <fstream>
#include <sstream>
#include <atomic>

using namespace std;

struct Task {
    int taskId;
    int requiredCPU;
    Task(int id, int cpu) : taskId(id), requiredCPU(cpu) {}
};

struct Node {
    int nodeId;
    bool isOnline;
    int cpuCapacity;
    int currentCPUUsage;
    vector<Task> tasks;

    Node(int id) {
        nodeId = id;
        isOnline = true;
        cpuCapacity = 10;
        currentCPUUsage = 0;
    }

    bool assignTask(const Task& task) {
        if (isOnline && currentCPUUsage + task.requiredCPU <= cpuCapacity) {
            tasks.push_back(task);
            currentCPUUsage += task.requiredCPU;
            return true;
        }
        return false;
    }

    void printStatus() const {
        cout << "Node " << nodeId << " | Status: "
             << (isOnline ? "Online" : "Offline")
             << " | CPU Usage: " << currentCPUUsage << "/" << cpuCapacity
             << " | Tasks: " << tasks.size() << endl;
    }
};

mutex logMutex;
atomic<int> taskCounter(1);
ofstream logFile("performance.log", ios::app);

// Log the task assignment
void logAssignment(int taskId, int cpu, int nodeId) {
    lock_guard<mutex> lock(logMutex);
    if (logFile.is_open()) {
        logFile << "Task " << taskId << " requiring " << cpu
                << " CPU assigned to Node " << nodeId << endl;
    }
}

void statusReport(const vector<Node>& nodes) {
    cout << "\n===== Node Status Report =====\n";
    for (const auto& node : nodes) {
        node.printStatus();
    }
    cout << "===============================\n";
}

bool assignTaskToNode(vector<Node>& nodes, int cpu) {
    int id = taskCounter.fetch_add(1);
    Task task(id, cpu);

    for (auto& node : nodes) {
        if (node.assignTask(task)) {
            cout << "Task " << id << " assigned to Node " << node.nodeId << "\n";
            logAssignment(id, cpu, node.nodeId);
            return true;
        }
    }

    cout << "Task " << id << " could not be assigned due to insufficient CPU.\n";
    return false;
}

int main() {
    vector<Node> nodes;
    for (int i = 1; i <= 3; ++i) {
        nodes.push_back(Node(i));
    }

    cout << "Cluster Resource Manager\n";
    cout << "Commands:\n";
    cout << "1. status\n";
    cout << "2. task <CPU>\n";
    cout << "3. exit\n";

    string input;
    while (true) {
        cout << "> ";
        getline(cin, input);
        stringstream ss(input);
        string command;
        ss >> command;

        if (command == "status") {
            statusReport(nodes);
        } else if (command == "task") {
            int cpu;
            if (ss >> cpu) {
                assignTaskToNode(nodes, cpu);
            } else {
                cout << "[ERROR] Invalid CPU input. Usage: task <CPU>\n";
            }
        } else if (command == "exit") {
            cout << "Exiting...\n";
            break;
        } else {
            cout << "[ERROR] Unknown command. Try: status | task <CPU> | exit\n";
        }
    }

    if (logFile.is_open()) logFile.close();
    return 0;
}
