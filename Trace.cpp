#include "Trace.hpp"
     // Copy constructor
Trace::Trace(std::string name) : thread(name) {
    fail = false;
}
Trace::~Trace() {
    lines.clear(); lines.resize(0);
}
void Trace::print(){
    std::cout << "thread " << thread << "stack trace: " << std::endl;
    for(std::string l : lines){
        std::cout << l << std::endl;
    }
}
