#ifndef AGENT_H
#define AGENT_H

#include <string>
#include <utility>

class Agent {
protected:
    int id;
    std::pair<int, int> position; // (x, y) coordinates
    std::string state;           // e.g., "IDLE", "MOVING", "CHARGING"

public:
    Agent(int agent_id, int x, int y) 
        : id(agent_id), position({x, y}), state("IDLE") {}
    
    virtual ~Agent() = default;

    // Getters
    int getId() const { return id; }
    std::pair<int, int> getPosition() const { return position; }
    std::string getState() const { return state; }

    // Setters
    void setPosition(int x, int y) { position = {x, y}; }
    void setState(const std::string& new_state) { state = new_state; }

    // Pure virtual methods to be overridden by derived classes
    virtual void update() = 0;
};

#endif // AGENT_H