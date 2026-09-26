#include "AStar.h"
#include <queue>
#include <map>
#include <algorithm>
#include <cmath>

static int heuristic(std::pair<int, int> a, std::pair<int, int> b) {
    return std::abs(a.first - b.first) + std::abs(a.second - b.second);
}

std::vector<std::pair<int, int>> AStar::findPath(
    std::pair<int, int> start,
    std::pair<int, int> goal,
    const Environment& env
) {
    std::vector<std::pair<int, int>> path;
    if (env.isObstacle(goal.first, goal.second)) return path;

    using Element = std::pair<int, std::pair<int, int>>; // {f_score, {x, y}}
    std::priority_queue<Element, std::vector<Element>, std::greater<Element>> pq;

    std::map<std::pair<int, int>, int> gScore;
    std::map<std::pair<int, int>, std::pair<int, int>> parent;

    gScore[start] = 0;
    pq.push(std::make_pair(heuristic(start, goal), start));

    int dx[] = {0, 0, 1, -1};
    int dy[] = {1, -1, 0, 0};

    while (!pq.empty()) {
        Element top = pq.top();
        pq.pop();

        int f = top.first;
        std::pair<int, int> current = top.second;

        if (current == goal) break;

        for (int i = 0; i < 4; ++i) {
            int nx = current.first + dx[i];
            int ny = current.second + dy[i];
            std::pair<int, int> nextPos = std::make_pair(nx, ny);

            if (env.isValidPosition(nx, ny) && !env.isObstacle(nx, ny)) {
                int tentative_g = gScore[current] + 1;

                if (gScore.find(nextPos) == gScore.end() || tentative_g < gScore[nextPos]) {
                    parent[nextPos] = current;
                    gScore[nextPos] = tentative_g;
                    int fScore = tentative_g + heuristic(nextPos, goal);
                    pq.push(std::make_pair(fScore, nextPos));
                }
            }
        }
    }

    if (gScore.find(goal) != gScore.end()) {
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