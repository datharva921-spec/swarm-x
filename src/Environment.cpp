#include "Environment.h"

Environment::Environment(int w, int h) : width(w), height(h) {
    grid = std::vector<std::vector<int>>(height, std::vector<int>(width, 0));
}

void Environment::addObstacle(int x, int y) {
    if (isValidPosition(x, y)) {
        grid[y][x] = 1;
    }
}

void Environment::removeObstacle(int x, int y) {
    if (isValidPosition(x, y)) {
        grid[y][x] = 0;
    }
}

void Environment::clearObstacles() {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            grid[y][x] = 0;
        }
    }
}

bool Environment::isObstacle(int x, int y) const {
    if (!isValidPosition(x, y)) return true;
    return grid[y][x] == 1;
}

bool Environment::isValidPosition(int x, int y) const {
    return x >= 0 && x < width && y >= 0 && y < height;
}

std::vector<std::pair<int, int>> Environment::getObstacles() const {
    std::vector<std::pair<int, int>> obstacles;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (grid[y][x] == 1) {
                obstacles.push_back({x, y});
            }
        }
    }
    return obstacles;
}