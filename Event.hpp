#ifndef EVENT_HPP
#define EVENT_HPP

#include <iostream>
#include <sstream> 
#include <string> 
#include <cstdio>

#include <vector>
#include <map>
#include <unordered_map>
/**
 * is a line of code
 */
class Event {
public:
    enum class EventType { // type of event
        undefined = 0,
        Condition,
        Location,
        Output,
        Invocation
    };

    int lineNum; // line number
    EventType type;
    Event* context;   
    std::string var; // variable (if Locational)
    std::string value; // variable value (if Locational) or true / false (if conditional)

    // Constructors
    Event(const int num);
    Event(const int num, EventType t);
    
    bool operator== (const Event& rhs) const;
    
private:

};

#endif // EVENT_HPP
