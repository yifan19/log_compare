#ifndef SRC_HPP
#define SRC_HPP

#include <memory>
#include <iostream>
#include <sstream> 
#include <string> 
#include <fstream> 
#include <cstdio>

#include <vector>
#include <map>
#include <unordered_map>
#include "globals.hpp"

class Node {
public:
    Node() {
        parent = nullptr;
    }
    ~Node() {}
    Node* parent;
};
class Variable{
public:
    int lineDef; // where defined
    std::string name;
    // std::string value;
    Variable(int line, std::string n);
};
class Invocation{
public:
    std::string name;
    Invocation(std::string n);
};

class IfBlock : public Node {
public:
    CE* condition;
    std::unique_ptr<Node> ifBranch;
    std::unique_ptr<Node> elseBranch;
    IfBlock() {}
    
};
class LoopBlock : public Node {
public:
    CE* condition;
    std::unique_ptr<Node> body;
    LoopBlock() {}
};

#endif // SRC_HPP
