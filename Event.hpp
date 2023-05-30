#ifndef EVENT_HPP
#define EVENT_HPP

#include <iostream>
#include <sstream> 
#include <string> 
#include <cstdio>

#include <vector>
#include <map>
#include <unordered_map>
#include "source.hpp"
/**
 * is a line of code
 */
class Event : public Node {
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

    // Constructors
    Event();
    Event(const int num);
    // Event(const int num, EventType t);
    
    bool operator== (const Event& rhs) const;
    bool operator!= (const Event& rhs) const;

};

class CE : public Event {
public:
    Event* context;   
    Expression* cond; // condition 
    bool value; // true / false 

    // Constructors
    CE();
    CE(const int num);
    CE(const int num, bool value);
    
    bool operator== (const Event& rhs) const;
    bool operator!= (const Event& rhs) const;

    bool evaluate();

};
class LE : public Event {
public:
    Event* context;   
    Variable* var; // condition 

    // Constructors
    LE();
    LE(const int num);
    
    bool operator== (const Event& rhs) const;
    bool operator!= (const Event& rhs) const;

};
#endif // EVENT_HPP
