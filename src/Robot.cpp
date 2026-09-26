#include "Robot.h"
#include <iostream>

void Robot::update() {
    // If the robot is busy with a task, consume battery each tick
    if (state == "BUSY" || state == "MOVING") {
        consumeBattery(0.5); // Consumes 0.5% battery per simulation tick
        
        if (getBattery() <= 0.0) {
            setState("OUT_OF_POWER");
            clearTask();
        }
    } else if (state == "CHARGING") {
        chargeBattery(2.0); // Charges 2.0% battery per simulation tick
        if (getBattery() >= 100.0) {
            setState("IDLE");
        }
    }
}