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
class Event{
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
    std::string value;

    // Constructors
    Event();
    Event(const int num);
    Event(const int num, const Event::EventType t);
    // Event(const int num, EventType t);
    
    bool operator== (const Event& rhs) const;
    bool operator!= (const Event& rhs) const;
    void print();
};

#endif