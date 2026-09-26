#include "Task.h"

std::string Task::getStatusString() const {
    switch (status) {
        case TaskStatus::PENDING: return "PENDING";
        case TaskStatus::IN_PROGRESS: return "IN_PROGRESS";
        case TaskStatus::COMPLETED: return "COMPLETED";
        case TaskStatus::FAILED: return "FAILED";
        default: return "UNKNOWN";
    }
}