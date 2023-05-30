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

Event* createLE(const int num, std::string variable){
    Event* LE = new Event(num, Event::EventType::Condition);
    LE->var = new Variable(num, variable);
    return LE;
}



    // Constructors
CE::CE(){
    type = Event::EventType::Condition;
    cond = nullptr;
}
CE::CE(const int num){
    type = Event::EventType::Condition;
    value = false;
    cond = new Expression(num);
}
CE::CE(const int num, bool val){
    type = Event::EventType::Condition;
    value = val;
    cond = new Expression(num);
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
CE::CE(const int num, bool val){
    type = Event::EventType::Condition;
    value = val;
    cond = new Expression(num);
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

