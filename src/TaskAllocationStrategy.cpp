#include "TaskAllocationStrategy.h"
#include <cmath>
#include <climits>
#include <algorithm>

static int getDistance(std::pair<int, int> a, std::pair<int, int> b) {
    return std::abs(a.first - b.first) + std::abs(a.second - b.second);
}

void GreedyTaskAllocationStrategy::allocateTasks(std::vector<Robot>& robots, std::vector<Task>& tasks) {
    for (auto& robot : robots) {
        if (robot.getState() != "IDLE" || robot.getBattery() <= 10.0) continue;

        int bestTaskIdx = -1;
        int minDistance = INT_MAX;

        for (size_t i = 0; i < tasks.size(); ++i) {
            if (tasks[i].getStatus() == TaskStatus::PENDING) {
                int dist = getDistance(robot.getPosition(), tasks[i].getLocation());
                if (dist < minDistance) {
                    minDistance = dist;
                    bestTaskIdx = static_cast<int>(i);
                }
            }
        }

        if (bestTaskIdx != -1) {
            tasks[bestTaskIdx].setStatus(TaskStatus::IN_PROGRESS);
            robot.assignTask(tasks[bestTaskIdx].getId());
        }
    }
}

void PriorityTaskAllocationStrategy::allocateTasks(std::vector<Robot>& robots, std::vector<Task>& tasks) {
    // Gather all pending tasks and sort by priority descending
    std::vector<size_t> pendingIndices;
    for (size_t i = 0; i < tasks.size(); ++i) {
        if (tasks[i].getStatus() == TaskStatus::PENDING) {
            pendingIndices.push_back(i);
        }
    }

    std::sort(pendingIndices.begin(), pendingIndices.end(), [&tasks](size_t a, size_t b) {
        return tasks[a].getPriority() > tasks[b].getPriority();
    });

    for (size_t taskIdx : pendingIndices) {
        int bestRobotIdx = -1;
        int minDistance = INT_MAX;

        for (size_t i = 0; i < robots.size(); ++i) {
            if (robots[i].getState() == "IDLE" && robots[i].getBattery() > 10.0) {
                int dist = getDistance(robots[i].getPosition(), tasks[taskIdx].getLocation());
                if (dist < minDistance) {
                    minDistance = dist;
                    bestRobotIdx = static_cast<int>(i);
                }
            }
        }

        if (bestRobotIdx != -1) {
            tasks[taskIdx].setStatus(TaskStatus::IN_PROGRESS);
            robots[bestRobotIdx].assignTask(tasks[taskIdx].getId());
        }
    }
}
