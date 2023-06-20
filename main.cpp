#include <iostream>
#include <sstream> 
#include <fstream>
#include <string> 
#include <cstdio>

#include <vector>
#include <map>
#include <unordered_map>

#include "Event.hpp"
#include "Log.hpp"
#include "globals.hpp"
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


std::pair<Log*, std::vector<Event>> logCompare(Log* failed, std::vector<Log*> succeeds){
    std::vector<Event> prefix; // longest common prefix
    if(succeeds.size()==0) {return std::make_pair(nullptr, prefix);}
    int max_length = 0; // longest prefix length
    int max_total = 0;
    int max_idx = 0;

    std::pair<Log*, std::vector<Event>> out;
    
    for(int i=0; i<succeeds.size(); i++){
        // int length = compare_one_log(failed, succeeds[i]);
        // std::cout << "comapring " << i << std::endl;
        // auto result = compare_log_contexts(failed, succeeds[i]);
        auto result = compare_log_maploops(failed, succeeds[i]);
        int length = result.first;
        if(length > max_length || (length == max_length && succeeds[i]->parsed.size() > max_total) ){
            out.first = succeeds[i];
            out.second = result.second;
            max_length = length; // update max length
            max_idx = i; // the run index in vector woth longest common prefix
            max_total = succeeds[i]->parsed.size();
        }
    }
    
    // int i = 0; Log lon(succeed_str[max_idx]); 
    // while(i<lon.current || !lon.parsedAll()){
    //     Event* es = lon.getEvent(i);
    //     if(es == nullptr){
    //         break;
    //     }else{        
    //         longest.push_back(*es);
    //     }
    //     i++;
    // }
    return out; //std::make_pair(max_length, succeeds[max_idx]);
} 

int main (int argc, char *argv[]){
    std::string filename = "step1a2.log";
    std::ifstream file1("logs/"+filename);
    if (!file1.is_open()) {
        std::cout << "Failed to open logs." << std::endl;
    }

    std::string newLogIndicator = "Method Entry";
    std::string failureIndicator = "BlockManager$ReplicationMonitor";
    
    
    std::string line; 
    // std::unordered_map<int, int> loopStarts; //= {{4, 0}, {1, 0}};
    // std::unordered_map <int, int> loopIds = {{4, 1},{3, 1},{2, 1},{1, 2}, {0, 2}};
    // std::unordered_map <int, int> loopStartIds = {{4, 1},{1, 2}}; std::unordered_map <int, int> parentLoop = {{1,-1}, {2,1}};
    
    std::vector<Log*> logs; // 
    
    std::unordered_map<int, Log*> threads;
    int num_fails = 0;
    int num_threads = 0; bool fail_encountered = false;
    while(std::getline(file1, line)){
        bool newLog = false; int thread = -1; bool fail = false;
        
        std::string::size_type temp_id = line.find("IPC Server handler ");
        if(temp_id != std::string::npos){ // deal with thread
            std::stringstream ss (line.substr(temp_id+19));
            ss >> thread; // thread number
        }
        else if(line.find(failureIndicator) != std::string::npos){ // failed run
            if(!fail_encountered){ // fail log did not appear before
                newLog = true; fail_encountered = true;
            }
            fail = true;
            thread = -2;
        }
        if(threads.find(thread) == threads.end()){ // new thread
            // threads[thread] = num_threads; num_threads++;
            newLog = true;
        }
        temp_id = line.find(newLogIndicator);
        if(temp_id != std::string::npos){ // new Log
            newLog = true;
            if(thread==-2) {num_fails++;}
        }
        
        Log* log = nullptr; 
        if(newLog){
            // std::cout << "new log! " << "fail: " << fail << std::endl;
            log = new Log();
            // log->loopIds = loopIds; 
            // log->loopStartIds = loopStartIds; log->loopIds_count = loopStartIds.size() + 1; log->parentLoop = parentLoop;
            // log->init_contexts(loopStarts);
            logs.push_back(log);
            threads[thread] = log; // new current log for that thread
            num_threads++;
        }else{
            log = threads[thread];
        }
        log->to_parse.push_back(line);
        log->fail = fail;
        // std::cout << line << ": thread # " << thread << " fail: " << fail << std::endl;
    }
    // std::cout << "# logs " << logs.size() << std::endl;

    std::vector<Log*> succeeds; std::vector<Log*> fails; 
    for(Log* l : logs){
        if(l==nullptr){
            std::cout << "null" << std::endl;
        }
        else if(l->fail){
            l->parseAll();
            fails.push_back(l);
        }else{
            l->parseAll();
            succeeds.push_back(l);
        }
    }
    
    if(fails.size()==0){
        std::cout << "no failed log found " << std::endl;
        return 1;
    }
    int k = 0;
//    std::cout << "failed run: " << std::endl;
//    fails[k]->printContexts();
//    fails[k]->printContexMaps();
//    std::cout << "good run: " << std::endl;
//    succeeds[1]->printContexts();
    // fails[k]->printLoops();
    auto result = logCompare(fails[k], succeeds);
    int length = result.second.size();
    std::cout << "length: " << (length) << ". ";
    for(int i=0; i<result.second.size(); i++){
        std::cout << result.second[i].idx << ":ID=" << result.second[i].lineNum << " ";
    }std::cout << std::endl;
    
//     auto result = logCompare(fails[k], succeeds);
//      //std::cout << max_idx ; // << " L" << fails[2]->getEvent(max_idx)->lineNum << std::endl;
    
    // fails[k]->printAll();
    std::cout << "prefix: " << std::endl;
    for(int i=0; i<length; i++){
        result.second[i].print();
    } std::cout << std::endl;
    if( length==fails[k]->parsed.size() && length==result.first->parsed.size() ){
          std::cout << "no divergence" << std::endl;
    }else{
        
        std::cout << "div at: " ; 
        if(length==0){ // div at the beginning
            if(fails[k]->parsed.size()>1 && fails[k]->getEvent(0)->lineNum==-1){
              std::cout << "HERE" << std::endl;
              fails[k]->getEvent(1)->print();
            }else{
                fails[k]->getEvent(0)->print();
            }
        }else if(length==1){
            if(fails[k]->parsed.size()>1 && result.second.back().lineNum==-1){ // the first one is entry
                fails[k]->getEvent(1)->print(); // print the first one after the entry
            }else{
                result.second.back().print();
            }
        }else {
            result.second.back().print();
        }
        std::cout << std::endl;
    }

//    fails[k]->printLoops();
//    for(auto it : fails[k]->loopStartIds){
//        std::cout << "{" << it.first << ", " << it.second << "} ";
//    } std::cout << std::endl;
    return 0;
}

/* seenIds
Failed Run: 
L2 val=3 L3 true L4 false L3 true L4 false L3 true L4 false L6 true L7 fail 
Successful Runs: 
Run 0: L2 val=3 L3 true L4 true L5 val=2 L3 true L4 true L5 val=1 L3 true L4 true L5 val=0 L6 false 
Run 1: L2 val=3 L3 true L4 trueseenIds L5 val=2 L3 true L4 false L3 true L4 false L3 true L4 true L5 val=1 L3 true L4 true L5 val=0 L6 false 
Run 2: L2 val=4 L3 true L4 false L3 true L4 true L5 val=3 L3 true L4 true L5 val=2 L3 true L4 true L5 val=1 L3 true L4 true L5 val=0 L6 false 

//// if we consider CE with different value (True / False) as different log:
Prefix: 
L2 L3 L4 L3 
Longest: 
L2 L3 L4 L3 L4 L5 L3 L4 L5 L3 L4 L5 L3 L4 L5 L6 
// diverge at L3

//// otherwise:
Prefix: 
L2 L3 L4 L3 L4 
Longest: 
L2 L3 L4 L3 L4 L5 L3 L4 L5 L3 L4 L5 L3 L4 L5 L6 
// diverge at L4
*/

