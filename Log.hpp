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
#include <stack>

#include "Event.hpp"

class Log {
public:
    std::deque<std::string> to_parse;
    std::vector<Event*> parsed;
    std::string entry; // Method entry
    std::stack<Event*> contextStack;
    std::unordered_map<int, int> loopStartLines;
    std::unordered_multimap<int, int> loopEndLines;
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
    void printContexts();
    bool failed();
    bool init_contexts(std::unordered_map<int, int>& start);
    bool init_contexts(std::unordered_map<int, int>& start, std::unordered_multimap<int, int> end);
};
int compare_one_log(Log* A, Log* B);
int compare_log_contexts(Log* A, Log* B);

#endif // LOG_HPP
