#ifndef COLLISION_AVOIDANCE_H
#define COLLISION_AVOIDANCE_H

#include <vector>
#include "Robot.h"

class CollisionAvoidance {
public:
    static bool checkCollision(const std::pair<int, int>& nextPos, int currentRobotId, const std::vector<Robot>& robots);
};

#endif // COLLISION_AVOIDANCE_H