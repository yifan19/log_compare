#include "Event.hpp"
#include "Log.hpp"

Log::Log(std::string log) {
    current = 0;
    to_parse.str(log);
}
     // Copy constructor
Log::Log(const Log& other) : current(other.current) {
    to_parse.str(other.to_parse.str());
    for (Event* e : other.parsed) {
        parsed.push_back(new Event(*e)); // deep copy
    }
}

Log& Log::operator=(const Log& rhs) {
    if (this != &rhs) { // not same object
        to_parse.str(rhs.to_parse.str());
        for (Event* e : parsed) {
            delete e;
        }
        parsed.clear();
        for (Event* e : rhs.parsed) {
            parsed.push_back(new Event(*e)); // deep copy 
        }
        current = rhs.current;
    }
    return *this;
}

Log::~Log(){
    for(int i=0; i<parsed.size(); i++){
        if(parsed[i] != nullptr){
            delete parsed[i];
        }
    }
    parsed.clear();
}

// Parse the next event from the log
Event* Log::parseNextEvent() {
    std::string line; std::string info; 
    std::string::size_type temp;
    if(to_parse >> line >> info){
        Event* e = new Event(std::stoi(line.substr(1))); // need to add error handling for this
        if(info.substr(info.size()-2) == "()"){ // Invocation
            e->type = Event::EventType::Invocation;
            e->value = info.substr(0, info.size()-2);
        }
        else if(info == "true" || info=="false"){ // condition
            e->type = Event::EventType::Condition;
            e->value = info;
        }
        else{
            temp = info.find('='); // variable = value
            if(temp != std::string::npos){
                e->type = Event::EventType::Location;
                e->var = info.substr(0, temp);
                e->value = info.substr(temp+1);
            }else{ // others (persumably output or "fail")
                e->type = Event::EventType::Output;
                e->value = info;
            }
        }
        parsed.push_back(e);
        current ++;
        return e;
    }else{ 
        return nullptr;
    }
}

// Check if there are more events in the log
bool Log::parsedAll() const{
    return to_parse.eof();
}

Event* Log::getEvent(int idx){
    if(idx < parsed.size()){ // which should equal current
        return parsed[idx];
    }
    Event* e = nullptr;
    while(!parsedAll()){
        e = parseNextEvent();
        if(current-1 == idx){
            return e;
        }
    }
    return nullptr;
}


