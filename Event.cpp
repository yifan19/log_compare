#include "Event.hpp"

Event::Event() {
    lineNum = -1; idx = -1;
    type = EventType::undefined; 
    context = nullptr; 
}
Event::Event(const int num) : lineNum(num) {
    type = EventType::undefined; idx = -1;
    context = nullptr; 
}
Event::Event(const int num, const Event::EventType t) : lineNum(num), type(t) {
    context = nullptr; idx = -1;
}
bool Event::operator== (const Event& rhs) const{
    if(lineNum != rhs.lineNum || type != rhs.type ){ // not same line number
        return false;
    }
    // if(type == EventType::Condition){
    //     // if conditional, value should be same
    //     if(value != rhs.value){
    //         // return false;
    //     }
    // }
    // should also compare context here
    return true;
}

bool Event::operator!= (const Event& rhs) const{
    return !(this->operator==(rhs));
}
void Event::print(){
    std::cout << "L" << lineNum << "=" << value ;
}
