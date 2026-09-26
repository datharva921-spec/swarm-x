#ifndef TASK_H
#define TASK_H

#include <utility>
#include <string>

enum class TaskStatus { PENDING, IN_PROGRESS, COMPLETED, FAILED };

class Task {
private:
    int taskId;
    std::pair<int, int> location;
    int priority; // Higher value = higher priority
    TaskStatus status;

public:
    Task(int id, int x, int y, int prio = 1)
        : taskId(id), location({x, y}), priority(prio), status(TaskStatus::PENDING) {}

    int getId() const { return taskId; }
    std::pair<int, int> getLocation() const { return location; }
    int getPriority() const { return priority; }
    TaskStatus getStatus() const { return status; }

    void setStatus(TaskStatus new_status) { status = new_status; }
    std::string getStatusString() const;
};

#endif // TASK_H