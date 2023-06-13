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


std::pair<int, Log*> logCompare(Log* failed, std::vector<Log*> succeeds){
    std::vector<Event> prefix; // longest common prefix
    if(succeeds.size()==0) {return std::make_pair(0, nullptr);}
    int max_length = 0; // longest prefix length
    int max_idx = 0;

    for(int i=0; i<succeeds.size(); i++){
        // int length = compare_one_log(failed, succeeds[i]);
        // std::cout << "comapring " << i << std::endl;
        auto result = compare_log_contexts(failed, succeeds[i]);
        int length = result.first;
        if(length > max_length){
            max_length = length; // update max length
            max_idx = i; // the run index in vector woth longest common prefix
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
    return std::make_pair(max_length, succeeds[max_idx]);
} 

int main (){
    std::ifstream file1("logs/all_logs.log");
    if (!file1.is_open()) {
        std::cout << "Failed to open logs." << std::endl;
    }

    std::string line; Log* log = nullptr; 
    std::vector<Log*> logs;
    std::unordered_map<int, int> loopStarts = {{1, 5}, {3, 5}};
    while(std::getline(file1, line)){
        if(line.find("Method Entry")!=std::string::npos){
            log = new Log();
            log->init_contexts(loopStarts);
            logs.push_back(log);
        }
        log->to_parse.push_back(line);
    }
    std::vector<Log*> fails;  std::vector<Log*> succeeds; 
    for(int i=0; i<logs.size(); i++){
        logs[i]->parseAll();
        if(logs[i]->failed()){fails.push_back(logs[i]);}
        else{succeeds.push_back(logs[i]);}
    }
    for(int i=0; i<fails.size(); i++){
        // fails[i]->printAll(); std::cout << "fail = " << fails[i]->fail << std::endl;
    }
    int k = 2;
    std::cout <<"failed contexts: " << std::endl;
    fails[k]->printContexts();
    std::cout << "succeeds[3] contexts: " << std::endl;
    succeeds[3]->printContexts();
    std::cout << "/////////// " << std::endl;

    
    // for(int i=0; i<succeeds.size(); i++){
    //     // int idx = compare_one_log(fails[2], succeeds[i]);
    //     std::cout << "i=" << i << ", ";
    //     for(auto iter : succeeds[i]->loopEndLines){
    //         std::cout << iter.first << ", " << iter.second << " ";
    //     }
    //     std::cout << std::endl;
    // }

    // for(int i=0; i<succeeds.size(); i++){
    //      succeeds[i]->printAll(); 
    //      std::cout << "fail = " << succeeds[i]->fail << ", ";
    //      int idx = compare_one_log(fails[k], succeeds[i]);
    //      std::cout << "length = " << idx << ".   ";
    //      std::cout << "div at: " ; fails[k]->getEvent(idx-1)->print();
    //      std::cout << std::endl;
    // }

     std::cout << "//// "; fails[k]->printAll();
     auto result = logCompare(fails[k], succeeds);
      //std::cout << max_idx ; // << " L" << fails[2]->getEvent(max_idx)->lineNum << std::endl;
      std::cout << "length: " << result.first << ". ";
      result.second->printAll();
      std::cout << "div at: " ; fails[k]->getEvent(result.first-1)->print();
      std::cout << std::endl;

      for(int i=0; i<fails[k]->parsed.size(); i++){
          std::cout << fails[k]->getEvent(i)->idx << ": " << fails[k]->getEvent(i)->lineNum << " ";
      } std::cout << std::endl;
      for(Event* c : fails[k]->parsed){
          std::cout << c->idx << ": L" << c->lineNum ;
          if(c->context != nullptr){std::cout << ": " << c->context->idx << ":L" << c->context->lineNum;}
          std::cout << std::endl;
          for(Event* e : fails[k]->contextMap[c->idx]){
              std::cout << e->idx<< " ";
          }std::cout << std::endl;
      } std::cout << std::endl;
    return 0;
}

/* // current output:
Failed Run: 
L2 val=3 L3 true L4 false L3 true L4 false L3 true L4 false L6 true L7 fail 
Successful Runs: 
Run 0: L2 val=3 L3 true L4 true L5 val=2 L3 true L4 true L5 val=1 L3 true L4 true L5 val=0 L6 false 
Run 1: L2 val=3 L3 true L4 true L5 val=2 L3 true L4 false L3 true L4 false L3 true L4 true L5 val=1 L3 true L4 true L5 val=0 L6 false 
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