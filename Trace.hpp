#ifndef TRACE_HPP
#define TRACE_HPP

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

class Trace {
public:
    std::vector<std::string> lines;
    std::string thread;

    bool fail;
    
    Trace() {}
    Trace(std::string name);
    ~Trace();

    void print();
};


#endif // TRACE_HPP
