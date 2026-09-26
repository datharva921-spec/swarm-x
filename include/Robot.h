#ifndef ROBOT_H
#define ROBOT_H

#include <algorithm>
#include "Agent.h"

class Robot : public Agent {
private:
    double batteryLevel; // 0.0 to 100.0%
    double speed;        // Units per step
    int currentTaskId;   // -1 if no task assigned

public:
    Robot(int agent_id, int x, int y, double initial_battery = 100.0, double agent_speed = 1.0)
        : Agent(agent_id, x, y), batteryLevel(initial_battery), speed(agent_speed), currentTaskId(-1) {}

    // Getters & Setters
    double getBattery() const { return batteryLevel; }
    double getSpeed() const { return speed; }
    void setSpeed(double s) { speed = s; }
    
    void consumeBattery(double amount) { batteryLevel = (std::max)(0.0, batteryLevel - amount); }
    void chargeBattery(double amount) { batteryLevel = (std::min)(100.0, batteryLevel + amount); }

    int getCurrentTaskId() const { return currentTaskId; }
    void assignTask(int taskId) { currentTaskId = taskId; state = "BUSY"; }
    void clearTask() { currentTaskId = -1; state = "IDLE"; }

    // Overridden pure virtual method from Agent
    void update() override;
};

#endif // ROBOT_H