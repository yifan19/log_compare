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

std::string compare_trace(Trace* t1, Trace* t2){
    int size1 = t1->lines.size();
    int size2 = t2->lines.size();
    int i=0;
    if(size1==0 || size2==0){
        return "";
    }
    for(; i<size1&&i<size2; i++){
        if(t1->lines[i] != t2->lines[i]){
            break;
        }
    }
    if(i>0){
        return t1->lines[i-1];
    }
    else{
        return t1->lines[0];
    }
}
