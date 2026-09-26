#include "CollisionAvoidance.h"

bool CollisionAvoidance::checkCollision(
    const std::pair<int, int>& nextPos, 
    int currentRobotId, 
    const std::vector<Robot>& robots
) {
    for (const auto& robot : robots) {
        if (robot.getId() != currentRobotId) {
            if (robot.getPosition() == nextPos) {
                return true; // Collision detected
            }
        }
    }
    return false; // Path clear
}