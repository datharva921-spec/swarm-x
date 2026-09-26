#ifndef ASTAR_H
#define ASTAR_H

#include "NavigationStrategy.h"

class AStar : public NavigationStrategy {
public:
    std::vector<std::pair<int, int>> findPath(
        std::pair<int, int> start,
        std::pair<int, int> goal,
        const Environment& env
    ) override;
};

#endif // ASTAR_H