#ifndef GLOBALS
#define GLOBALS

#include <iostream>
#include <sstream> 
#include <string> 
#include <cstdio>

#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>

#include "Event.hpp"
#include "source.hpp"
#include "Log.hpp"

extern Node* root;
extern std::vector<Node*> nodes;
extern std::map<std::string, Variable*> variables; // names as keys
std::unordered_set<std::string> java_var_types = {"int", "boolean", "String", "double", "char", "float", "long", "short", "byte"};

#endif // GLOBALS