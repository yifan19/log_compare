#ifndef LOG_HPP
#define LOG_HPP

#include <iostream>
#include <sstream> 
#include <string> 
#include <cstdio>

#include <vector>
#include <map>
#include <unordered_map>

#include "Event.hpp"
#include "source.hpp"

class Log {
public:
    std::stringstream to_parse;
    std::vector<Event*> parsed;
    int current; // current number of parsed events. current-1 is last index
    // Initialize log with a string stream
    Log(std::string log);
     // Copy constructor
    Log(const Log& other);

    Log& operator=(const Log& rhs);

    ~Log();

    // Parse the next event from the log
    Event* parseNextEvent();

    // Check if there are more events in the log
    bool parsedAll() const;

    Event* getEvent(int idx);

};

#endif // LOG_HPP
