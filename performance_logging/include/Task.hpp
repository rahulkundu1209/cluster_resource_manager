#ifndef TASK_HPP
#define TASK_HPP

struct Task {
    int id;
    int requiredCPU;      // in percentage (0–100)
    int requiredMemory;   // in MB
    int requiredIO;       // in MB/s or IOPS (arbitrary unit for simulation)

    Task(int id, int cpu, int mem, int io)
        : id(id), requiredCPU(cpu), requiredMemory(mem), requiredIO(io) {}
};

#endif
