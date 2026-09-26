#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "NavigationStrategy.h"

class Dijkstra : public NavigationStrategy {
public:
    std::vector<std::pair<int, int>> findPath(
        std::pair<int, int> start,
        std::pair<int, int> goal,
        const Environment& env
    ) override;
};

#endif // DIJKSTRA_H