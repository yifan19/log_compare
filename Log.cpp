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
        e->idx = parsed.size();

        auto range = loopEndLines.equal_range(e->lineNum);
        for(auto it = range.first; it!=range.second; it++){
            contextStack.pop();
            if(contextStack.top() != nullptr && e->lineNum > contextStack.top()->lineNum){
                std::cout << "pop " << e->lineNum << " " << contextStack.top()->lineNum << std::endl;
                contextStack.pop();
            }
        }
        e->context = contextStack.top();
        if(loopStartLines.find(e->lineNum) != loopStartLines.end()){
            
                contextStack.push(e);
        }
        if(e->context != nullptr){
            contextMap[e->context->idx].insert(e);
            int idx = parsed.size()-1;
            if(idx >= 0){
               contextMap[idx].insert(e);
            }
        }
    }
    parsed.push_back(e);
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
// int compare_log_contexts(Log* A, Log* B){
//     // int match_length = 0; int max_length = 0;
//     A->parseAll(); B->parseAll();
//     std::cout << "HERE" << std::endl;
//     int nA = A->parsed.size(); int nB = B->parsed.size();
//     std::vector<std::vector<int>> DP(nA, std::vector<int>(nB, 0)); 
//     Event* eA = nullptr; Event* eB = nullptr;
//     for(int idxA = 1; idxA < nA; idxA++){
//         for(int idxB = 1; idxB < nB; idxB++){
//             eA = A->parsed[idxA-1]; eB = B->parsed[idxB-1];
//             if(eA==nullptr || eB==nullptr){
//                 std::cout << "Null event detected at A[" << idxA-1 << "] or B[" << idxB-1 << "].\n";
//                 continue;
//             }
//             if(eA->lineNum == eB->lineNum){
//                 if(eA->context==nullptr && eB->context==nullptr){
//                     DP[idxA][idxB] = DP[idxA-1][idxB-1] + 1;
//                 }else if(eA->context!=nullptr && eB->context!=nullptr 
//                     && eA->context->lineNum == eB->context->lineNum){
//                     DP[idxA][idxB] = DP[idxA-1][idxB-1] + 1;
//                 }
//             }else if(DP[idxA-1][idxB] < DP[idxA][idxB-1]){
//                 DP[idxA][idxB] = DP[idxA][idxB-1];
//             } else {
//                 DP[idxA][idxB] = DP[idxA-1][idxB];
//             }
//         }
//     }
//     // std::cout << ":: " << DP[nA-1][nB-1] << std::endl;
//     int max = 0;
//     for(int i=0; i<nA; i++){
//         for(int j=0; j<nB; j++){
//             //std::cout << DP[i][j] << " ";
//             if(DP[i][j] > max){
//                 max = DP[i][j];
//             }
//         }
//         // std::cout << std::endl;
//     }
//     return max;
// }

// int compare_log_contexts(Log* A, Log* B){
//     A->parseAll(); B->parseAll();
//     int nA = A->parsed.size(); int nB = B->parsed.size();
//     std::vector<std::vector<int>> DP(nA, std::vector<int>(nB, 0)); 

//     for(int idxA = 0; idxA < nA; idxA++){
//         for(int idxB = 0; idxB < nB; idxB++){
//             Event* eA = A->parsed[idxA]; 
//             Event* eB = B->parsed[idxB]; 
//             if(eA->lineNum == eB->lineNum){
//                 if((eA->context == nullptr && eB->context == nullptr)
//                     || (eA->context != nullptr && eB->context != nullptr && eA->context->lineNum == eB->context->lineNum)){
//                     DP[idxA][idxB] = (idxA > 0 && idxB > 0 ? DP[idxA-1][idxB-1] : 0) + 1;
//                 } else if (idxA > 0 && idxB > 0){
//                     DP[idxA][idxB] = std::max(DP[idxA-1][idxB], DP[idxA][idxB-1]);
//                 }
//             } else {
//                 if(idxA > 0) DP[idxA][idxB] = DP[idxA-1][idxB];
//                 if(idxB > 0 && DP[idxA][idxB] < DP[idxA][idxB-1]) DP[idxA][idxB] = DP[idxA][idxB-1];
//             }
//         }
//     }
//     std::cout << ":: " << DP[nA-1][nB-1] << std::endl;
//     for(int i=0; i<nA; i++){
//         for(int j=0; j<nB; j++){
//             std::cout << DP[i][j] << " ";
//         }
//         std::cout << std::endl;
//     }
//     return nA > 0 && nB > 0 ? DP[nA-1][nB-1] : 0; 
// }

bool Log::failed(){
    parseAll();
    return fail;
}
bool Log::init_contexts(std::unordered_map<int, int>& start){
    loopStartLines = start; // loopEndLines = end;
    for(auto i : loopStartLines){
        loopEndLines.insert({i.second, i.first});
    }
    Event* root = new Event(0); 
    contextStack.push(root);
    return (loopStartLines.size() == loopEndLines.size());
}
bool Log::init_contexts(std::unordered_map<int, int>& start, std::unordered_multimap<int, int> end){
    loopStartLines = start; loopEndLines = end;
    contextStack.push(nullptr);
    return (loopStartLines.size() == loopEndLines.size());
}
void dfs(Log* A, Log* B, int idxA, int idxB, int commonLength, int &maxCommonLength, std::unordered_map<int, 
std::vector<Event*>> &contextMap, std::vector<std::vector<bool>> &visited);

int dfs(Event* rootA, Event* rootB, std::unordered_map<int, std::vector<Event*>>& mapA, std::unordered_map<int, std::vector<Event*>>& mapB){
    std::vector<Event*> childrenA = mapA[rootA->lineNum];
    std::vector<Event*> childrenB = mapB[rootB->lineNum];
    for(auto i : childrenA){
       //  std::cout << "ii " << i->lineNum << std::endl;
    }
    int max_length = 0;
    rootA->print(); std::cout << "!" << std::endl;
    for(Event* eA : childrenA){
        for(Event*eB : childrenB){
            // eA->print(); std::cout << "-" << std::endl;
            // eB->print(); std::cout << "-" << std::endl;
            if(eA!=nullptr && eB!=nullptr && eA->lineNum==eB->lineNum){
                if((eA->context==nullptr && eB->context==nullptr) ||
                    (eA->context!=nullptr && eB->context!=nullptr && eA->context->lineNum==eB->context->lineNum)){
                    std::cout << "HERE!" << std::endl;    
                    int curr_length = 1 + dfs(eA, eB, mapA, mapB);
                    std::cout << "curr " << curr_length << std::endl;
                    max_length = std::max(max_length, curr_length);
                }
            }
        }
    }
    std::cout << "max " << max_length << " ";
    return max_length; 
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

    if(result.first > length){
        for(Event* e : result.second){
            prefix.push_back(*e);
        }
        return std::make_pair(length, prefix);
    }
        
        for(int i=0; i<length; i++){
            prefix.push_back(*A->getEvent(i));
        }
        return std::make_pair(length, prefix);

}

