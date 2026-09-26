#ifndef SIMULATION_ENGINE_H
#define SIMULATION_ENGINE_H

#include <vector>
#include <memory>
#include <string>
#include "Environment.h"
#include "Robot.h"
#include "Task.h"
#include "NavigationStrategy.h"
#include "TaskAllocationStrategy.h"

class SimulationEngine {
private:
    Environment environment;
    std::vector<Robot> initialRobots;
    std::vector<Task> initialTasks;
    std::vector<Robot> robots;
    std::vector<Task> tasks;
    std::unique_ptr<NavigationStrategy> activePathfinder;
    std::unique_ptr<TaskAllocationStrategy> activeTaskAllocator;
    std::string activeAlgorithmName;
    int currentTick;

public:
    SimulationEngine(int mapWidth, int mapHeight);

    void setNavigationStrategy(std::unique_ptr<NavigationStrategy> strategy, const std::string& name);
    bool setNavigationStrategyByName(const std::string& name);
    std::string getActiveNavigationStrategyName() const { return activeAlgorithmName; }

    void setTaskAllocationStrategy(std::unique_ptr<TaskAllocationStrategy> strategy);

    void addRobot(const Robot& robot);
    void addTask(const Task& task);

    void addObstacle(int x, int y);
    void removeObstacle(int x, int y);
    void clearObstacles();

    void reset();
    void step(); // Advance simulation state by 1 tick
    
    // JSON Exporter for Web Frontend Visualizer & Dashboard
    std::string getJSONState() const; 
};

#endif // SIMULATION_ENGINE_H