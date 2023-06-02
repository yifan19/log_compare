#ifndef LOG_HPP
#define LOG_HPP

#include <iostream>
#include <sstream> 
#include <string> 
#include <cstdio>

#include <vector>
#include <map>
#include <unordered_map>
#include <deque>
#include <regex>

#include "Event.hpp"

class Log {
public:
    std::deque<std::string> to_parse;
    std::vector<Event*> parsed;
    std::string entry; // Method entry
    bool fail;
    // Initialize log with a string stream
    Log() {}
     // Copy constructor
    Log(const Log& other);

    Log& operator=(const Log& rhs);

    ~Log();

    // Parse the next event from the log
    Event* parseNextLine();

    Event* getEvent(int idx);
    bool parseAll();
    void printParsed();
    void printAll();
    bool failed();
};
int compare_one_log(Log* A, Log* B);

#endif // LOG_HPP
