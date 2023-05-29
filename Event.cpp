#include "Event.hpp"
#include "source.hpp"

Event::Event(const int num) : lineNum(num) {
    type = EventType::undefined; 
    context = nullptr; 
    var = nullptr;
    value = false;
}
Event::Event(const int num, EventType t) : lineNum(num), type(t) {
    context = nullptr; 
    var = nullptr;
    value = false;
}

bool Event::operator== (const Event& rhs) const{
    if(lineNum != rhs.lineNum || type != rhs.type ){ // not same line number
        return false;
    }
    if(type == EventType::Invocation){
        // if invocational, call should be same
        if(var->lineDef != rhs.var->lineDef){
            return false;
        }
    }
    if(type == EventType::Condition){
        // if conditional, value should be same
        if(value != rhs.value){
            // return false;
        }
    }
    // should also compare context here
    return true;
}

bool Event::operator!= (const Event& rhs) const{
    return !(this->operator==(rhs));
}
