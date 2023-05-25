#include <iostream>
#include <sstream> 
#include <string> 
#include <cstdio>

#include <vector>
#include <map>
#include <unordered_map>

std::string chooseRandom(int init, const std::vector<bool> &cond){ // L1
    std::stringstream log; log << std::boolalpha;
    int val = init; // L2
    log << "L2 " << "val=" << val << " "; // L2 instrument: val
    int loop = cond.size(); // number of loops
    
    for(int i=0; i<loop; i++){ // L3: loop condition
        log << "L3 " << (i<loop) << " "; // L3 instrument: loop condition
        log << "L4 " << cond[i] << " "; // L4 instrument: if condition
        if(cond[i]){ // L4: if condition
            val--; // L5 
            log << "L5 " << "val=" << val << " "; // L5 instrument: val change
        }
    }
    log << "L6 " << (val > 0) << " "; // L6 instrument: if condition
    if(val > 0){ // L6: if condition
        log << "L7 " << "fail "; // L7 instrument: throw Exception
    }
    return log.str();
}

/**
 * is a line of code
 */
class Event {
public:
    enum class EventType { // type of event
        undefined = 0,
        Condition,
        Location,
        Output,
        Invocation
    };

    int lineNum; // line number
    EventType type;
    Event* context;   
    std::string var; // variable (if Locational)
    std::string value; // variable value (if Locational) or true / false (if conditional)

    // Constructors
    Event(const int num) : lineNum(num) {type = EventType::undefined; context = nullptr; var = "";}
    Event(const int num, EventType t) : lineNum(num), type(t) {context = nullptr; var = "";}
    
    bool operator== (const Event& rhs) const{
        if(lineNum != rhs.lineNum || type != rhs.type ){ // not same line number
            return false;
        }
        if(type == EventType::Condition || type == EventType::Invocation){
            // if conditional / invocational, value should be same
            if(value != rhs.value){
                return false;
            }
        }
        // should also compare context here
        return true;
    }
private:

};

class Log {
public:
    std::stringstream to_parse;
    std::vector<Event*> parsed;
    int current; // current number of parsed events. current-1 is last index
    // Initialize log with a string stream
    Log(std::string log) {
        current = 0;
        to_parse.str(log);
    }
    ~Log(){
        for(int i=0; i<parsed.size(); i++){
            if(parsed[i] != nullptr){
                delete parsed[i];
            }
        }
        parsed.clear();
    }

    // Parse the next event from the log
    Event* parseNextEvent() {
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
    bool parsedAll() const{
        return to_parse.eof();
    }

    Event* getEvent(int idx){
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

};


    
std::pair<int, std::vector<Event>> logCompare(std::string failed_str, std::vector<std::string>& succeed_str){
    Log failed(failed_str);
    // std::unordered_map<int, Log> succeeds;
    int max_length = 0; // longest prefix length
    int max_idx = -1;
    for(int i=0; i<succeed_str.size(); i++){
        Log succeed(succeed_str[i]);
        int idx = 0; int length = 0; 
        while(failed.getEvent(idx)!=nullptr && succeed.getEvent(idx)!=nullptr){
            Event* ef = failed.getEvent(idx); // parse as we proceed
            Event* es = succeed.getEvent(idx); // might be able to store this some where
            if(*es == *ef){ // compare lineNum
                length++;
                if(length > max_length){
                    max_length = length; // update max length
                    max_idx = i; // the run index in vector woth longest common prefix
                }
            }else{
                break; // diverge
            }
            idx++;
        }
    }
    std::vector<Event> prefix; // longest common prefix
    for (int i = 0; i < max_length; i++) {
        Event* ef = failed.getEvent(i);
        prefix.push_back(*ef); // reconstruct common prefix from failed
    }
    return std::make_pair(max_idx, prefix);
} 


int main (){
    int init = 3;
    std::vector<bool> cond = {false, true, true};
    std::cout << std::boolalpha;
    std::string failed_str = chooseRandom(init, cond);
    std::cout << "Failed Run: " << std::endl << failed_str << std::endl;

    std::cout << "Successful Runs: " << std::endl;
    std::vector<std::string> succeeds;
    init = 3; cond = {true, true, true};
    succeeds.push_back(chooseRandom(init, cond));
    init = 3; cond = {true, false, false, true, true};
    succeeds.push_back(chooseRandom(init, cond));
    init = 4; cond = {false, true, true, true, true};
    succeeds.push_back(chooseRandom(init, cond));
    for(int i=0; i<succeeds.size(); i++){
        std::cout << "Run " << i << ": " << succeeds[i] << std::endl;
    }

    auto result = logCompare(failed_str, succeeds);
    std::cout << "Max prefix run #: " << (result.first) << std::endl;
    std::vector<Event> prefix = result.second;
    for(int i=0; i<prefix.size(); i++){
        std::cout << "L" << prefix[i].lineNum << " ";
    } std::cout << std::endl;

    return 0;
}

