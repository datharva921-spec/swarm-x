#include "SimulationEngine.h"
#include "BFS.h"
#include "Dijkstra.h"
#include "AStar.h"
#include "CollisionAvoidance.h"
#include <sstream>
#include <algorithm>

SimulationEngine::SimulationEngine(int mapWidth, int mapHeight)
    : environment(mapWidth, mapHeight), currentTick(0) {
    // Default strategy: A* for heuristic shortest path
    activeAlgorithmName = "AStar";
    activePathfinder = std::make_unique<AStar>();
    // Default task allocation: Priority-based nearest robot
    activeTaskAllocator = std::make_unique<PriorityTaskAllocationStrategy>();
}

void SimulationEngine::setNavigationStrategy(std::unique_ptr<NavigationStrategy> strategy, const std::string& name) {
    activePathfinder = std::move(strategy);
    activeAlgorithmName = name;
}

bool SimulationEngine::setNavigationStrategyByName(const std::string& name) {
    if (name == "BFS") {
        setNavigationStrategy(std::make_unique<BFS>(), "BFS");
        return true;
    } else if (name == "Dijkstra") {
        setNavigationStrategy(std::make_unique<Dijkstra>(), "Dijkstra");
        return true;
    } else if (name == "AStar" || name == "A*") {
        setNavigationStrategy(std::make_unique<AStar>(), "AStar");
        return true;
    }
    return false;
}

void SimulationEngine::setTaskAllocationStrategy(std::unique_ptr<TaskAllocationStrategy> strategy) {
    activeTaskAllocator = std::move(strategy);
}

void SimulationEngine::addRobot(const Robot& robot) {
    robots.push_back(robot);
    initialRobots.push_back(robot);
}

void SimulationEngine::addTask(const Task& task) {
    tasks.push_back(task);
    initialTasks.push_back(task);
}

void SimulationEngine::addObstacle(int x, int y) {
    environment.addObstacle(x, y);
}

void SimulationEngine::removeObstacle(int x, int y) {
    environment.removeObstacle(x, y);
}

void SimulationEngine::clearObstacles() {
    environment.clearObstacles();
}

void SimulationEngine::reset() {
    currentTick = 0;
    robots = initialRobots;
    tasks = initialTasks;
}

void SimulationEngine::step() {
    currentTick++;

    // 1. Allocate pending tasks to available idle robots
    if (activeTaskAllocator) {
        activeTaskAllocator->allocateTasks(robots, tasks);
    }

    // 2. Process each robot's motion and state update
    for (auto& robot : robots) {
        if (robot.getBattery() <= 0.0) continue;

        if (robot.getCurrentTaskId() != -1) {
            // Locate assigned task target
            std::pair<int, int> target = robot.getPosition();
            bool taskFound = false;
            for (const auto& task : tasks) {
                if (task.getId() == robot.getCurrentTaskId()) {
                    target = task.getLocation();
                    taskFound = true;
                    break;
                }
            }

            if (!taskFound) {
                robot.clearTask();
                continue;
            }

            // If already at target, complete task
            if (robot.getPosition() == target) {
                for (auto& task : tasks) {
                    if (task.getId() == robot.getCurrentTaskId()) {
                        task.setStatus(TaskStatus::COMPLETED);
                        break;
                    }
                }
                robot.clearTask();
            } else {
                // Find path using active navigation strategy
                auto path = activePathfinder->findPath(robot.getPosition(), target, environment);

                if (path.size() > 1) {
                    std::pair<int, int> nextStep = path[1];

                    // Collision avoidance check against other robots
                    if (!CollisionAvoidance::checkCollision(nextStep, robot.getId(), robots)) {
                        robot.setPosition(nextStep.first, nextStep.second);

                        // Check if robot just arrived at target
                        if (robot.getPosition() == target) {
                            for (auto& task : tasks) {
                                if (task.getId() == robot.getCurrentTaskId()) {
                                    task.setStatus(TaskStatus::COMPLETED);
                                    break;
                                }
                            }
                            robot.clearTask();
                        }
                    }
                }
            }
        }

        robot.update();
    }
}

std::string SimulationEngine::getJSONState() const {
    std::stringstream ss;
    ss << "{";
    ss << "\"tick\":" << currentTick << ",";
    ss << "\"algorithm\":\"" << activeAlgorithmName << "\",";
    ss << "\"grid\":{\"width\":" << environment.getWidth() << ",\"height\":" << environment.getHeight() << "},";

    // Serialize Obstacles
    auto obstacles = environment.getObstacles();
    ss << "\"obstacles\":[";
    for (size_t i = 0; i < obstacles.size(); ++i) {
        ss << "[" << obstacles[i].first << "," << obstacles[i].second << "]";
        if (i + 1 < obstacles.size()) ss << ",";
    }
    ss << "],";

    // Serialize Robots with calculated path trajectory for visualization
    ss << "\"robots\":[";
    for (size_t i = 0; i < robots.size(); ++i) {
        ss << "{"
           << "\"id\":" << robots[i].getId() << ","
           << "\"x\":" << robots[i].getPosition().first << ","
           << "\"y\":" << robots[i].getPosition().second << ","
           << "\"battery\":" << robots[i].getBattery() << ","
           << "\"state\":\"" << robots[i].getState() << "\","
           << "\"taskId\":" << robots[i].getCurrentTaskId() << ",";

        // Calculate preview path to target if assigned
        ss << "\"path\":[";
        if (robots[i].getCurrentTaskId() != -1 && activePathfinder) {
            std::pair<int, int> target = robots[i].getPosition();
            for (const auto& task : tasks) {
                if (task.getId() == robots[i].getCurrentTaskId()) {
                    target = task.getLocation();
                    break;
                }
            }
            auto p = activePathfinder->findPath(robots[i].getPosition(), target, environment);
            for (size_t k = 0; k < p.size(); ++k) {
                ss << "[" << p[k].first << "," << p[k].second << "]";
                if (k + 1 < p.size()) ss << ",";
            }
        }
        ss << "]";

        ss << "}";
        if (i + 1 < robots.size()) ss << ",";
    }
    ss << "],";

    // Serialize Tasks
    int pendingCount = 0;
    int inProgressCount = 0;
    int completedCount = 0;

    ss << "\"tasks\":[";
    for (size_t i = 0; i < tasks.size(); ++i) {
        if (tasks[i].getStatus() == TaskStatus::PENDING) pendingCount++;
        else if (tasks[i].getStatus() == TaskStatus::IN_PROGRESS) inProgressCount++;
        else if (tasks[i].getStatus() == TaskStatus::COMPLETED) completedCount++;

        ss << "{"
           << "\"id\":" << tasks[i].getId() << ","
           << "\"x\":" << tasks[i].getLocation().first << ","
           << "\"y\":" << tasks[i].getLocation().second << ","
           << "\"priority\":" << tasks[i].getPriority() << ","
           << "\"status\":\"" << tasks[i].getStatusString() << "\","
           << "\"status_code\":" << static_cast<int>(tasks[i].getStatus())
           << "}";
        if (i + 1 < tasks.size()) ss << ",";
    }
    ss << "],";

    // Serialize Telemetry / Statistics
    ss << "\"stats\":{"
       << "\"totalTasks\":" << tasks.size() << ","
       << "\"completedTasks\":" << completedCount << ","
       << "\"pendingTasks\":" << pendingCount << ","
       << "\"inProgressTasks\":" << inProgressCount << ","
       << "\"activeRobots\":" << robots.size()
       << "}";

    ss << "}";
    return ss.str();
}