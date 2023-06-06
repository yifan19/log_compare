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
    if(e!=nullptr){
        e->context = contextStack.top();
        if(loopStartLines.find(e->lineNum) != loopStartLines.end()){
            contextStack.push(e);
        }
        auto range = loopEndLines.equal_range(e->lineNum);
        for(auto it = range.first; it!=range.second; it++){
            contextStack.pop();
        }
        if(e->context != nullptr){
            contextMap[e->context].push_back(e);
        }
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
       parsed[i]->print(); 
       std::cout << ", " ;
    }
    std::cout << std::endl;
}

void Log::printAll(){
    parseAll();
    printParsed();
}
void Log::printContexts(){
    for(int i=1; i<parsed.size(); i++){
       parsed[i]->print(); 
       if(parsed[i]->context == nullptr){
            std::cout << " ctx: -1";
        }else{
            std::cout << " ctx: " << parsed[i]->context->lineNum;
        }
       std::cout << ", " ;
    }
    std::cout << std::endl;
}

int compare_one_log(Log* A, Log* B){
    int idx = 0; 
    int nA = A->parsed.size() + A->to_parse.size();
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
int compare_log_contexts(Log* A, Log* B){
    // int match_length = 0; int max_length = 0;
    A->parseAll(); B->parseAll();
    int nA = A->parsed.size(); int nB = B->parsed.size();
    std::vector<std::vector<int>> DP(nA+1, std::vector<int>(nB+1, 0)); 

    for(int idxA = 1; idxA <= nA; idxA++){
        for(int idxB = 1; idxB <= nB; idxB++){
            if(A->parsed[idxA-1]->lineNum == B->parsed[idxB-1]->lineNum
                && A->parsed[idxA-1]->context->lineNum == B->parsed[idxB-1]->context->lineNum){
                    DP[idxA][idxB] = DP[idxA-1][idxB-1] + 1;
            } else if(DP[idxA-1][idxB] < DP[idxA][idxB-1]){
                DP[idxA][idxB] = DP[idxA][idxB-1];
            } else {
                DP[idxA][idxB] = DP[idxA-1][idxB];
            }
        }
    }
    return DP[nA][nB];
}

bool Log::failed(){
    parseAll();
    return fail;
}
bool Log::init_contexts(std::unordered_map<int, int>& start){
    loopStartLines = start; // loopEndLines = end;
    for(auto i : loopStartLines){
        loopEndLines.insert({i.second, i.first});
    }
    contextStack.push(nullptr);
    return (loopStartLines.size() == loopEndLines.size());
}
bool Log::init_contexts(std::unordered_map<int, int>& start, std::unordered_multimap<int, int> end){
    loopStartLines = start; loopEndLines = end;
    contextStack.push(nullptr);
    return (loopStartLines.size() == loopEndLines.size());
}

