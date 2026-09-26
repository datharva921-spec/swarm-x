#include "BFS.h"
#include <queue>
#include <map>
#include <algorithm>

std::vector<std::pair<int, int>> BFS::findPath(
    std::pair<int, int> start,
    std::pair<int, int> goal,
    const Environment& env
) {
    std::vector<std::pair<int, int>> path;
    if (env.isObstacle(goal.first, goal.second)) return path;

    std::queue<std::pair<int, int>> q;
    std::map<std::pair<int, int>, std::pair<int, int>> parent;
    std::map<std::pair<int, int>, bool> visited;

    q.push(start);
    visited[start] = true;

    // Standard 4-directional grid movement (Up, Down, Left, Right)
    int dx[] = {0, 0, 1, -1};
    int dy[] = {1, -1, 0, 0};

    bool found = false;

    while (!q.empty()) {
        auto current = q.front();
        q.pop();

        if (current == goal) {
            found = true;
            break;
        }

        for (int i = 0; i < 4; ++i) {
            int nx = current.first + dx[i];
            int ny = current.second + dy[i];

            std::pair<int, int> nextPos = {nx, ny};

            if (env.isValidPosition(nx, ny) && !env.isObstacle(nx, ny) && !visited[nextPos]) {
                visited[nextPos] = true;
                parent[nextPos] = current;
                q.push(nextPos);
            }
        }
    }

    if (found) {
        std::pair<int, int> curr = goal;
        while (curr != start) {
            path.push_back(curr);
            curr = parent[curr];
        }
        path.push_back(start);
        std::reverse(path.begin(), path.end());
    }

    return path;
}