#ifndef SRC_HPP
#define SRC_HPP

#include <memory>
#include <iostream>
#include <sstream> 
#include <string> 
#include <cstdio>

#include <vector>
#include <map>
#include <unordered_map>

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

class Expression : public Node {
public:
    int lineNum;
    std::vector<Variable*> variables;
    Expression(int line);
};

class IfBlock : public Node {
public:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Node> ifBranch;
    std::unique_ptr<Node> elseBranch;
    IfBlock() {}
    
};
class LoopBlock : public Node {
public:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Node> body;
    LoopBlock() {}
};

#endif // SRC_HPP
