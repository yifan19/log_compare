#include "Event.hpp"
#include "source.hpp"

Event::Event() {
    lineNum = -1;
    type = EventType::undefined; 
    context = nullptr; 
}
Event::Event(const int num) : lineNum(num) {
    type = EventType::undefined; 
    context = nullptr; 
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


// Constructors
CE::CE(){
    type = Event::EventType::Condition;
}
CE::CE(const int num){
    type = Event::EventType::Condition;
    value = false;
}
CE::CE(const int num, bool val){
    type = Event::EventType::Condition;
    value = val;
}
bool CE::operator== (const Event& rhs) const{
    if(lineNum != rhs.lineNum){
        return false;
    }
    return true;
}
bool CE::operator!= (const Event& rhs) const{
    return !(this->operator==(rhs));
}

bool CE::evaluate(){
    return value;
}

LE::LE(){
    type = Event::EventType::Location;
    var = nullptr;
}
LE::LE(const int num){
    type = Event::EventType::Location;
    var = nullptr;
}

bool LE::operator== (const Event& rhs) const{
    if(lineNum != rhs.lineNum){
        return false;
    }
    return true;
}
bool LE::operator!= (const Event& rhs) const{
    return !(this->operator==(rhs));
}

