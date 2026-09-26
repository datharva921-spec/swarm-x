#include "Dijkstra.h"
#include <queue>
#include <map>
#include <algorithm>
#include <climits>

std::vector<std::pair<int, int>> Dijkstra::findPath(
    std::pair<int, int> start,
    std::pair<int, int> goal,
    const Environment& env
) {
    std::vector<std::pair<int, int>> path;
    if (env.isObstacle(goal.first, goal.second)) return path;

    using Element = std::pair<int, std::pair<int, int>>; // {cost, {x, y}}
    std::priority_queue<Element, std::vector<Element>, std::greater<Element>> pq;

    std::map<std::pair<int, int>, int> dist;
    std::map<std::pair<int, int>, std::pair<int, int>> parent;

    pq.push(std::make_pair(0, start));
    dist[start] = 0;

    int dx[] = {0, 0, 1, -1};
    int dy[] = {1, -1, 0, 0};

    while (!pq.empty()) {
        Element top = pq.top();
        pq.pop();

        int d = top.first;
        std::pair<int, int> current = top.second;

        if (current == goal) break;
        if (d > dist[current]) continue;

        for (int i = 0; i < 4; ++i) {
            int nx = current.first + dx[i];
            int ny = current.second + dy[i];
            std::pair<int, int> nextPos = {nx, ny};

            if (env.isValidPosition(nx, ny) && !env.isObstacle(nx, ny)) {
                int newDist = dist[current] + 1; // Movement weight = 1 per cell
                if (dist.find(nextPos) == dist.end() || newDist < dist[nextPos]) {
                    dist[nextPos] = newDist;
                    parent[nextPos] = current;
                    pq.push(std::make_pair(newDist, nextPos));
                }
            }
        }
    }

    if (dist.find(goal) != dist.end()) {
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