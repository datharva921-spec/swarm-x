#ifndef BFS_H
#define BFS_H

#include "NavigationStrategy.h"

class BFS : public NavigationStrategy {
public:
    std::vector<std::pair<int, int>> findPath(
        std::pair<int, int> start,
        std::pair<int, int> goal,
        const Environment& env
    ) override;
};

#endif // BFS_H