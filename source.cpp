#include "source.hpp"

Variable::Variable(int line, std::string n) : lineDef(line), name(n) {}

/// 
Invocation::Invocation(std::string n) : name(n) {}

/// 
Expression::Expression(int line) : lineNum(line) {}

//
bool parseJava(){
    // make something up
}


