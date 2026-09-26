#ifndef TASK_ALLOCATION_STRATEGY_H
#define TASK_ALLOCATION_STRATEGY_H

#include <vector>
#include <memory>
#include "Robot.h"
#include "Task.h"

class TaskAllocationStrategy {
public:
    virtual ~TaskAllocationStrategy() = default;
    virtual void allocateTasks(std::vector<Robot>& robots, std::vector<Task>& tasks) = 0;
};

// Allocates closest pending task to each idle robot (FIFO / Proximity)
class GreedyTaskAllocationStrategy : public TaskAllocationStrategy {
public:
    void allocateTasks(std::vector<Robot>& robots, std::vector<Task>& tasks) override;
};

// Allocates highest priority pending tasks first to nearest available robots
class PriorityTaskAllocationStrategy : public TaskAllocationStrategy {
public:
    void allocateTasks(std::vector<Robot>& robots, std::vector<Task>& tasks) override;
};

#endif // TASK_ALLOCATION_STRATEGY_H