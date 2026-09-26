#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <string>
#include <cstdlib>
#include "httplib.h"
#include "SimulationEngine.h"

int main() {
    std::cout << "\n=======================================================\n";
    std::cout << "           SWARM-X MULTI-AGENT SIMULATION ENGINE       \n";
    std::cout << "=======================================================\n";
    std::cout << " [System] Initializing 10x10 Grid Environment...\n";

    // 1. Initialize Simulation Engine (10x10 Grid)
    SimulationEngine engine(10, 10);

    // 2. Add Warehouse Obstacles / Partitions
    engine.addObstacle(3, 2);
    engine.addObstacle(3, 3);
    engine.addObstacle(3, 4);
    engine.addObstacle(3, 5);

    engine.addObstacle(6, 4);
    engine.addObstacle(6, 5);
    engine.addObstacle(6, 6);
    engine.addObstacle(6, 7);

    // 3. Add Initial Autonomous Robots (id, x, y, battery, speed)
    engine.addRobot(Robot(1, 0, 0, 100.0, 1.0));
    engine.addRobot(Robot(2, 9, 0, 100.0, 1.0));
    engine.addRobot(Robot(3, 0, 9, 100.0, 1.0));

    // 4. Add Initial Tasks (id, x, y, priority)
    engine.addTask(Task(101, 5, 5, 3));
    engine.addTask(Task(102, 2, 8, 2));
    engine.addTask(Task(103, 8, 7, 1));
    engine.addTask(Task(104, 7, 2, 4));
    engine.addTask(Task(105, 4, 9, 2));

    std::cout << " [System] 3 Autonomous Robots and 5 Tasks initialized.\n";
    std::cout << " [System] Navigation Strategy: A* (Heuristic Search).\n";

    // 5. Set Up HTTP Web Server
    httplib::Server svr;

    // Serve static frontend files from web/ directory
    bool mountSuccess = svr.set_mount_point("/", "./web");
    if (!mountSuccess) {
        std::cerr << " [Warning] 'web' directory not found relative to working directory.\n";
    }

    // API: Get current simulation state
    svr.Get("/api/state", [&engine](const httplib::Request&, httplib::Response& res) {
        res.set_content(engine.getJSONState(), "application/json");
    });

    // API: Advance simulation 1 tick and return state
    svr.Get("/api/tick", [&engine](const httplib::Request&, httplib::Response& res) {
        engine.step();
        res.set_content(engine.getJSONState(), "application/json");
    });

    // API: Switch navigation algorithm (BFS, Dijkstra, AStar)
    auto handleStrategy = [&engine](const httplib::Request& req, httplib::Response& res) {
        std::string algo = req.get_param_value("algo");
        if (algo.empty()) algo = "AStar";
        engine.setNavigationStrategyByName(algo);
        std::cout << " [Control] Navigation algorithm switched to: " << engine.getActiveNavigationStrategyName() << "\n";
        res.set_content(engine.getJSONState(), "application/json");
    };
    svr.Get("/api/strategy", handleStrategy);
    svr.Post("/api/strategy", handleStrategy);

    // API: Reset simulation to initial state
    auto handleReset = [&engine](const httplib::Request&, httplib::Response& res) {
        engine.reset();
        std::cout << " [Control] Simulation reset to initial state.\n";
        res.set_content(engine.getJSONState(), "application/json");
    };
    svr.Get("/api/reset", handleReset);
    svr.Post("/api/reset", handleReset);

    // API: Add Task dynamically
    auto handleAddTask = [&engine](const httplib::Request& req, httplib::Response& res) {
        std::string sx = req.get_param_value("x");
        std::string sy = req.get_param_value("y");
        std::string sp = req.get_param_value("priority");

        int x = sx.empty() ? 5 : std::atoi(sx.c_str());
        int y = sy.empty() ? 5 : std::atoi(sy.c_str());
        int prio = sp.empty() ? 1 : std::atoi(sp.c_str());

        static int nextTaskId = 200;
        engine.addTask(Task(nextTaskId++, x, y, prio));
        std::cout << " [Task] New task added at (" << x << ", " << y << ") with priority " << prio << "\n";
        res.set_content(engine.getJSONState(), "application/json");
    };
    svr.Get("/api/add_task", handleAddTask);
    svr.Post("/api/add_task", handleAddTask);

    // API: Add or Remove Obstacle dynamically
    auto handleObstacle = [&engine](const httplib::Request& req, httplib::Response& res) {
        std::string sx = req.get_param_value("x");
        std::string sy = req.get_param_value("y");
        std::string action = req.get_param_value("action");

        int x = sx.empty() ? 0 : std::atoi(sx.c_str());
        int y = sy.empty() ? 0 : std::atoi(sy.c_str());

        if (action == "remove") {
            engine.removeObstacle(x, y);
        } else if (action == "clear") {
            engine.clearObstacles();
        } else {
            engine.addObstacle(x, y);
        }
        res.set_content(engine.getJSONState(), "application/json");
    };
    svr.Get("/api/obstacle", handleObstacle);
    svr.Post("/api/obstacle", handleObstacle);

    std::cout << "\n-------------------------------------------------------\n";
    std::cout << " [Server] SWARM-X Web Dashboard running at:\n";
    std::cout << "          >>> http://localhost:8080 <<<\n";
    std::cout << "-------------------------------------------------------\n";
    std::cout << " [Info] Press Ctrl+C in terminal to stop server.\n\n";

    // 6. Start Web Server
    svr.listen("0.0.0.0", 8080);

    return 0;
}