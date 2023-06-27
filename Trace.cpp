#include "Trace.hpp"
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
