#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <vector>
#include <utility>

class Environment {
private:
    int width;
    int height;
    std::vector<std::vector<int>> grid; // 0 = empty, 1 = obstacle

public:
    Environment(int w, int h);

    int getWidth() const { return width; }
    int getHeight() const { return height; }

    void addObstacle(int x, int y);
    void removeObstacle(int x, int y);
    void clearObstacles();
    bool isObstacle(int x, int y) const;
    bool isValidPosition(int x, int y) const;

    const std::vector<std::vector<int>>& getGrid() const { return grid; }
    std::vector<std::pair<int, int>> getObstacles() const;
};

#endif // ENVIRONMENT_H