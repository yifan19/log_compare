#include "Event.hpp"
#include "Log.hpp"

     // Copy constructor
Log::Log(const Log& other) {
    to_parse = other.to_parse;
    parsed = other.parsed;
    entry = other.entry;
    loopIds_count = 0;
    contexts[-1] = nullptr;
}

Log& Log::operator=(const Log& rhs) {
    if (this != &rhs) { // not same object
        to_parse = rhs.to_parse;
        parsed = rhs.parsed;
        entry = rhs.entry;
        loopIds = rhs.loopIds;
        loopStartIds = rhs.loopStartIds;
        // loopEndIds = rhs.loopEndIds;
        loopIds_count = rhs.loopIds_count;
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
    
    temp_id = line.find("Method Entry");
    if(temp_id != std::string::npos){ // is Method Entry
        // std::cout << "HERE" << std::endl;
        line = line.substr(temp_id+14);
        entry = line;
        temp_id = line.find("(");
        e = new Event();
        e->type = Event::EventType::Invocation;
        if(temp_id != std::string::npos){
            e->value = line.substr(0, temp_id);
            line = line.substr(temp_id+1);
        }
        temp_id = line.find(":");
        if(temp_id != std::string::npos){
            std::stringstream ss(line.substr(temp_id+1));
            int id=-1; ss >> id;
            e->lineNum = id; 
        }
        // std::cout << "HERE " << e->lineNum << std::endl;
    }
    // is ID=
    temp_id = line.find("ID="); // std::cout << "ID! " << std::endl;
    if(temp_id != std::string::npos){
        line = line.substr(temp_id+3);
        temp_id = line.find(",");
        int id = std::stoi(line.substr(0, temp_id)); // between ID and ,
        if(e!=nullptr) {
            e->lineNum = id;
        }else{
            e = new Event(id);
        }
        
        if(temp_id != std::string::npos){ // after ,
            e->value = line.substr(temp_id+1);
            if(e->value=="true" || e->value=="false"){
            e->type = Event::EventType::Condition;
            }else if(is_number(e->value)){
                e->type = Event::EventType::Location;
            }else{
                e->type = Event::EventType::Output;
            }
        }

    }
    
    to_parse.pop_front();
    if(e==nullptr){
        std::cout << "parse fail: " << line << std::endl;
        return nullptr;
    }
    // if(e!=nullptr && e->lineNum==6 && e->value=="true"){
    //     fail = true;
    // }
    
    e->idx = parsed.size();
    if(e->idx ==0){
        contexts[-1] = e;
    }
    auto it = loopIds.find(e->lineNum);
    if(it!=loopIds.end()){ // is in a loop
        e->loopId = it->second;
        if(contexts.find(e->loopId)!=contexts.end()){ 
            e->context = contexts[e->loopId];
        }
    }

    it = loopStartIds.find(e->lineNum);
    if(it!=loopStartIds.end()){ // is the start of a loop
        e->startLoopId = it->second;
        if(contexts.find(e->loopId)!=contexts.end()){
            e->context = contexts[e->loopId]; // context is last loop
        }else{
            if(parentLoop.find(e->loopId)!=parentLoop.end())
                {e->context = contexts[parentLoop[e->loopId]];} // contexts of those in start of loops
            else
                {e->context = contexts[-1];}
        }
        contexts[e->startLoopId] = e; // update the context of a loopId
    }
    
    if(e->context != nullptr){
        contextMap[e->context->idx].insert(e);
    }
    if(e->idx>0){
        contextMap[e->idx-1].insert(e);
    }
    parsed.push_back(e); // std::cout << "parsed " << e->lineNum << std::endl;
    return e;
}

bool Log::set_contexts(std::vector<int>& contexts, int n){
    parseAll();
    int size = parsed.size();
    for(int i=0; i<n&&i<size; i++){
        parsed[i]->context = parsed[contexts[i]];
    }
    return true;
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



int compare_one_log(Log* A, Log* B){
    int idx = 0; 
    int nA = A->parsed.size() + A->to_parse.size();
    int nB = B->parsed.size() + B->to_parse.size();
    //std::cout << "nA " << nA << " nB" << nB << std::endl;
    while((idx < nA) && (idx < nB)){
        // std::cout << "idx=" << idx << " " ;
        Event* ef = A->getEvent(idx); 
        Event* es = B->getEvent(idx); 
        // ef->print(); std::cout << " "; es->print(); std::cout << std::endl;
        // std::cout << "es " << es->lineNum << " ef " << ef->lineNum << " idx " << idx << std::endl ;
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

std::pair<int, std::vector<Event*>> bfs_start(Log* A, Log* B){
    // std::vector<Event> prefix;
    A->parseAll(); B->parseAll();
    int sizeA = A->parsed.size(); int sizeB = B->parsed.size();
    std::vector<Event*> current;
    if(sizeA==0 || sizeB==0) {return std::make_pair(0, current);}

    std::queue<Node> q;
    q.push({A->getEvent(0), B->getEvent(0), 1});
    
    int length = 0;

    while (!q.empty()) {
        Node node = q.front(); 
        q.pop();
        if(node.eventA==nullptr || node.eventB==nullptr){
            continue;
        }
        if (*(node.eventA) == *(node.eventB)) {
            if (node.depth > length) {
                length = node.depth;
                current.push_back(node.eventA);
            }
                // Enqueue children
            auto childrenA = A->contextMap[node.eventA->idx];
            auto childrenB = B->contextMap[node.eventB->idx];
            
            for (auto& childA : childrenA) {
                // For each child in A, if it also exists in B, enqueue them
                for (auto& childB : childrenB) {
                    if (*childA == *childB) {
                        q.push({childA, childB, node.depth+1});
                    }
                }
            }
        }
    }
    return std::make_pair(length, current);
}
std::pair<int, std::vector<Event>> compare_log_contexts(Log* A, Log* B){
    int length = compare_one_log(A, B);
    std::vector<Event> prefix;

    auto result = bfs_start(A, B);
    // std::cout << "bfs " << result.first << std::endl;
    // for(Event* e : result.second){
    //     std::cout << e->idx << ":L" << e->lineNum << " ";
    // }std::cout << std::endl;

    // if(result.first > length){
        for(Event* e : result.second){
            prefix.push_back(*e);
        }
        return std::make_pair(length, prefix);
    // }
        
//    for(int i=0; i<length; i++){ 
//        prefix.push_back(*A->getEvent(i));
//    }
//    return std::make_pair(length, prefix);

}


std::pair<int, std::vector<Event>> compare_log_maploops(Log* A, Log* B){
//    int length = compare_one_log(A, B);
    std::vector<Event> prefix;
   
    auto result = loop_dfs(A, B);
    int length = result.first;
    
    // if(result.first > length){
        for(Event* e : result.second){
            prefix.push_back(*e);
        }
        return std::make_pair(length, prefix);

        
//    for(int i=0; i<length; i++){
//        prefix.push_back(*A->getEvent(i));
//    }
//    return std::make_pair(length, prefix);

}

std::pair<int, std::vector<Event*>> loop_dfs(Log* A, Log* B){
    // std::vector<Event> prefix;
    A->parseAll(); B->parseAll();
    int sizeA = A->parsed.size(); int sizeB = B->parsed.size();
    std::vector<Event*> current;
    if(sizeA==0 || sizeB==0) {return std::make_pair(0, current);}

    std::queue<Node> q;
    q.push({A->getEvent(0), B->getEvent(0), 1});
    
    int length = 0;

    while (!q.empty()) {
        Node node = q.front(); 
        q.pop();
        if(node.eventA==nullptr || node.eventB==nullptr){
            continue;
        }
        if (*(node.eventA) == *(node.eventB)) {
            if (node.depth > length) {
                length = node.depth;
                current.push_back(node.eventA);
            }
            // Enqueue 
            int idxA = node.eventA->idx; int idxB = node.eventB->idx; 
            std::vector<Event*> childrenA; 
            Event* nextA = A->getEvent(idxA+1);
            if(nextA!=nullptr){
                childrenA.push_back(nextA); // next event
                if(nextA->startLoopId != -2){ // starting or immediately after ending a loop
                    for(int i=idxA+2; i<A->parsed.size(); i++){
                        if(A->parsed[i]->startLoopId == nextA->startLoopId){
                            childrenA.push_back(A->parsed[i]); 
                        }
                    }
                }
            }
                    
            std::vector<Event*> childrenB; Event* nextB = B->getEvent(idxB+1);
            if(nextB!=nullptr){
                childrenB.push_back(nextB); // next event
                if(nextB->startLoopId != -2){ // starting or immediately after ending a loop
                    for(int i=idxB+2; i<B->parsed.size(); i++){
                        if(B->parsed[i]->startLoopId == nextB->startLoopId){
                            childrenB.push_back(B->parsed[i]); 
                        }
                    }
                }
            }
            
            for (auto& childA : childrenA) {
                // For each child in A, if it also exists in B, enqueue them
                for (auto& childB : childrenB) {
                    if (*childA == *childB) {
                        q.push({childA, childB, node.depth+1});
                    }
                }
            }
        }
    }
    return std::make_pair(length, current);
}

std::pair<int, std::vector<Event*>> context_bfs(Log* A, Log* B){
    // std::vector<Event> prefix;
    A->parseAll(); B->parseAll();
    int sizeA = A->parsed.size(); int sizeB = B->parsed.size();
    std::vector<Event*> current;
    if(sizeA==0 || sizeB==0) {return std::make_pair(0, current);}

    std::queue<Node> q;
    q.push({A->getEvent(0), B->getEvent(0), 1});
    
    int length = 0;

    while (!q.empty()) {
        Node node = q.front(); 
        q.pop();
        if(node.eventA==nullptr || node.eventB==nullptr){
            continue;
        }
        if (*(node.eventA) == *(node.eventB)) {
            if (node.depth > length) {
                length = node.depth;
                current.push_back(node.eventA);
            }
            // Enqueue 
            int idxA = node.eventA->idx; int idxB = node.eventB->idx; 
            std::vector<Event*> childrenA; 
            Event* nextA = A->getEvent(idxA+1);
            if(nextA!=nullptr){
                childrenA.push_back(nextA); // next event
                if(nextA->startLoopId != -2){ // starting or immediately after ending a loop
                    for(int i=idxA+2; i<A->parsed.size(); i++){
                        if(A->parsed[i]->startLoopId == nextA->startLoopId){
                            childrenA.push_back(A->parsed[i]); 
                        }
                    }
                }
            }
                    
            std::vector<Event*> childrenB; Event* nextB = B->getEvent(idxB+1);
            if(nextB!=nullptr){
                childrenB.push_back(nextB); // next event
                if(nextB->startLoopId != -2){ // starting or immediately after ending a loop
                    for(int i=idxB+2; i<B->parsed.size(); i++){
                        if(B->parsed[i]->startLoopId == nextB->startLoopId){
                            childrenB.push_back(B->parsed[i]); 
                        }
                    }
                }
            }
            
            for (auto& childA : childrenA) {
                // For each child in A, if it also exists in B, enqueue them
                for (auto& childB : childrenB) {
                    if (*childA == *childB) {
                        q.push({childA, childB, node.depth+1});
                    }
                }
            }
        }
    }
    return std::make_pair(length, current);
}

////////////// PRINTS //////////////////////////////////////////////

void Log::printParsed(){
    for(int i=0; i<parsed.size(); i++){
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
            std::cout << " ctx: ";
            parsed[i]->context->print();
        }
       std::cout << std::endl ;
    }
    std::cout << std::endl;
}
void Log::printContexMaps(){
    for(auto it : contextMap){
        if(it.first>=0){
            getEvent(it.first)->print();
        }else{
            std::cout << it.first;
        }
        std::cout << ": " << std::endl << "-> ";
        for(Event* e : it.second){
            e->print(); std::cout << " ";
        }
        std::cout << std::endl;
    }
}
void Log::printLoops(){
    for(int i=0; i<parsed.size(); i++){
       parsed[i]->print(); 
       std::cout << " loop id: " << parsed[i]->loopId;
       std::cout << ", " ;
    }
    std::cout << std::endl;
}
