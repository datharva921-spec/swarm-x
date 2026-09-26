#ifndef NAVIGATION_STRATEGY_H
#define NAVIGATION_STRATEGY_H

#include <vector>
#include <utility>
#include "Environment.h"

class NavigationStrategy {
public:
    virtual ~NavigationStrategy() = default;

    // Takes start (x,y), goal (x,y), and grid map, returning a list of path coordinates
    virtual std::vector<std::pair<int, int>> findPath(
        std::pair<int, int> start,
        std::pair<int, int> goal,
        const Environment& env
    ) = 0;
};

#endif // NAVIGATION_STRATEGY_H