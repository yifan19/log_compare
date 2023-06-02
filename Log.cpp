#include "Event.hpp"
#include "Log.hpp"

     // Copy constructor
Log::Log(const Log& other) {
    to_parse = other.to_parse;
    parsed = other.parsed;
    entry = other.entry;
}

Log& Log::operator=(const Log& rhs) {
    if (this != &rhs) { // not same object
        to_parse = rhs.to_parse;
        parsed = rhs.parsed;
        entry = rhs.entry;
    }
    return *this;
}

Log::~Log(){
    for(int i=0; i<parsed.size(); i++){
        if(parsed[i] != nullptr){
            delete parsed[i];
        }
    }
    to_parse.clear();
    parsed.clear();
}
static const std::regex numRegex("^[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?$");
bool is_number(std::string str){   
    return std::regex_match(str, numRegex);
}
Event* Log::parseNextLine() {
    if(to_parse.empty()){
        return nullptr;
    }
    Event* e = nullptr;

    std::string::size_type temp_id;
    std::string line = to_parse.front();
    temp_id = line.find("[Method Entry]");
     
    if(temp_id != std::string::npos){ // is Method Entry
        line = line.substr(temp_id+14);
        entry = line;
        temp_id = line.find("(");
        e = new Event();
        e->type = Event::EventType::Invocation;
        e->value = line.substr(0, temp_id);
        line = line.substr(temp_id+1);

        temp_id = line.find(":");
        std::stringstream ss(line.substr(temp_id+1));
        int id=-1; ss >> id;
        e->lineNum = id; 
        parsed.push_back(e);
    }else{ // is ID=
        temp_id = line.find("ID=");
        if(temp_id != std::string::npos){
            line = line.substr(temp_id+3);
            temp_id = line.find(",");
            int id = std::stoi(line.substr(0, temp_id));
            e = new Event(id);
            if(temp_id != std::string::npos){
                e->value = line.substr(temp_id+1);
                if(e->value=="true" || e->value=="false"){
                    e->type = Event::EventType::Condition;
                }else if(is_number(e->value)){
                    e->type = Event::EventType::Location;
                }else{
                    e->type = Event::EventType::Output;
                }
            }
            e->lineNum = id; 
            parsed.push_back(e);
        }else{
            //something is wrong
            std::cout << "ID not found" << std::endl;
        }
    }
    to_parse.pop_front();
    if(e!=nullptr && e->lineNum==6 && e->value=="true"){
        fail = true;
    }
    return e;
}

Event* Log::getEvent(int idx){
    if(idx < 0 || idx >= (parsed.size()+to_parse.size())) {return nullptr;}
    int n = parsed.size();
    if(idx < n){ // which should equal current
        return parsed[idx];
    }
    Event* e = nullptr;
    for(int i=n; i<=idx; i++){
       e = parseNextLine();
    }
    return e;
}

bool Log::parseAll(){
    while(!to_parse.empty()){
        parseNextLine();
    }
    return (to_parse.size()==0);
}

void Log::printParsed(){
    for(int i=1; i<parsed.size(); i++){
       parsed[i]->print(); std::cout << ", " ;
    }
    std::cout << std::endl;
}
void Log::printAll(){
    parseAll();
    printParsed();
}

int compare_one_log(Log* A, Log* B){
    int idx = 0; int nA = A->parsed.size() + A->to_parse.size();
    int nB = B->parsed.size() + B->to_parse.size();
    //std::cout << "nA " << nA << " nB" << nB << std::endl;
    while((idx < nA-1) && (idx < nB-1)){
        // std::cout << "idx=" << idx << " " ;
        Event* ef = A->getEvent(idx); 
        Event* es = B->getEvent(idx); 
        // ef->print(); std::cout << " "; es->print(); std::cout << std::endl;
        if(*es != *ef){ // compare lineNum
            break; // diverge
        }
        idx++;
    }
    //std::cout << "return " << idx << std::endl;
    return idx; // length of common prefix
}

bool Log::failed(){
    parseAll();
    return fail;
}

