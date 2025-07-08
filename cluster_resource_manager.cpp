#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <cstdlib>
#include <ctime>

using namespace std;

// ==============================
// Structs
// ==============================

struct Task {
    int taskId;
    int requiredCPU;
    int requiredMemory;
    int assignedNode;
    bool isCompleted;

    Task(int id, int cpu, int mem, int node = -1, bool done = false)
        : taskId(id), requiredCPU(cpu), requiredMemory(mem), assignedNode(node), isCompleted(done) {}
};

struct Node {
    int nodeId;
    int totalCPU;
    int totalMemory;
    int availableCPU;
    int availableMemory;
    bool isActive = true;
    bool isFailed = false;

    Node(int id, int cpu, int mem)
        : nodeId(id), totalCPU(cpu), totalMemory(mem),
          availableCPU(cpu), availableMemory(mem) {}
};

// ==============================
// Globals
// ==============================

vector<Node> clusterNodes;
vector<Task> runningTasks;
mutex nodeMutex;

// ==============================
// Task Allocation Logic
// ==============================

bool assignTaskToNode(Task& task) {
    lock_guard<mutex> lock(nodeMutex);
    for (auto& node : clusterNodes) {
        if (node.isActive && !node.isFailed &&
            node.availableCPU >= task.requiredCPU &&
            node.availableMemory >= task.requiredMemory) {
            
            node.availableCPU -= task.requiredCPU;
            node.availableMemory -= task.requiredMemory;
            task.assignedNode = node.nodeId;
            cout << "[+] Task " << task.taskId << " assigned to Node " << node.nodeId << endl;
            return true;
        }
    }
    return false;
}

// ==============================
// Simulate Task Execution
// ==============================

void simulateTaskExecution(Task task) {
    {
        lock_guard<mutex> lock(nodeMutex);
        cout << "[~] Task " << task.taskId << " running on Node " << task.assignedNode << endl;
    }

    this_thread::sleep_for(chrono::seconds(2));  // simulate task time

    lock_guard<mutex> lock(nodeMutex);
    Node& node = clusterNodes[task.assignedNode];
    node.availableCPU += task.requiredCPU;
    node.availableMemory += task.requiredMemory;

    task.isCompleted = true;
    cout << "[✓] Task " << task.taskId << " completed on Node " << task.assignedNode << endl;

    // Remove task from running list
    for (auto it = runningTasks.begin(); it != runningTasks.end(); ++it) {
        if (it->taskId == task.taskId) {
            runningTasks.erase(it);
            break;
        }
    }
}

// ==============================
// Task Handler
// ==============================

void handleNewTask(Task task) {
    if (assignTaskToNode(task)) {
        {
            lock_guard<mutex> lock(nodeMutex);
            runningTasks.push_back(task);
        }
        thread t(simulateTaskExecution, task);
        t.detach();
    } else {
        cout << "[-] Task " << task.taskId << " could not be assigned.\n";
    }
}

// ==============================
// Simulate Node Failure
// ==============================

void simulateNodeFailure() {
    lock_guard<mutex> lock(nodeMutex);
    for (auto& node : clusterNodes) {
        if (node.isActive && !node.isFailed && rand() % 10 == 0) {
            node.isFailed = true;
            node.isActive = false;
            cout << "[X] Node " << node.nodeId << " has FAILED!\n";
        }
    }
}

// ==============================
// Failover Logic
// ==============================

void performFailover() {
    lock_guard<mutex> lock(nodeMutex);

    vector<Task> toReassign;

    for (auto& task : runningTasks) {
        if (!task.isCompleted && task.assignedNode != -1) {
            Node& node = clusterNodes[task.assignedNode];
            if (node.isFailed) {
                cout << "[!] Task " << task.taskId << " was on failed Node " << node.nodeId << ", reassigning...\n";
                node.availableCPU += task.requiredCPU;
                node.availableMemory += task.requiredMemory;
                task.assignedNode = -1;
                toReassign.push_back(task);
            }
        }
    }

    for (auto& task : toReassign) {
        if (assignTaskToNode(task)) {
            thread t(simulateTaskExecution, task);
            t.detach();
        } else {
            cout << "[!] Task " << task.taskId << " could not be reassigned.\n";
        }
    }
}

// ==============================
// Main
// ==============================

int main() {
    srand(time(0));
    cout << "=== Cluster Resource Manager Simulation ===" << endl;

    // Initialize Nodes
    clusterNodes.push_back(Node(0, 100, 200));
    clusterNodes.push_back(Node(1, 80, 150));
    clusterNodes.push_back(Node(2, 120, 180));

    // Simulate Task Submissions
    for (int i = 0; i < 10; ++i) {
        Task t(i, 20 + (i % 3) * 10, 40 + (i % 2) * 30);
        handleNewTask(t);

        simulateNodeFailure();  // randomly fail nodes
        performFailover();      // recover tasks

        this_thread::sleep_for(chrono::milliseconds(500));
    }

    // Wait for tasks to finish
    this_thread::sleep_for(chrono::seconds(10));

    cout << "\n=== Simulation End ===" << endl;
    return 0;
}
