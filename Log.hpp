#ifndef LOG_HPP
#define LOG_HPP

#include <iostream>
#include <sstream> 
#include <string> 
#include <cstdio>

#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <regex>
#include <stack>
#include <queue>

#include "Event.hpp"

class Log {
public:
    std::deque<std::string> to_parse;
    std::vector<Event*> parsed;
    std::string entry; // Method entry
    std::stack<Event*> contextStack;
    std::unordered_map<int, int> loopStartLines;
    std::unordered_map<int, Event*> firstLoop;
    std::unordered_multimap<int, int> loopEndLines;
    std::unordered_map<int, std::unordered_set<Event*>> contextMap; // idx in the Log, not lineNum

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
std::pair<int, std::vector<Event*>> bfs_start(Log* A, Log* B);
std::pair<int, std::vector<Event>> compare_log_contexts(Log* A, Log* B);
typedef struct node {
    Event* eventA;
    Event* eventB;
    int depth;
} Node;
#endif // LOG_HPP
