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
    int max_total = 0;
    int max_idx = 0;

    for(int i=0; i<succeeds.size(); i++){
        // int length = compare_one_log(failed, succeeds[i]);
        // std::cout << "comapring " << i << std::endl;
        // auto result = compare_log_contexts(failed, succeeds[i]);
        auto result = compare_one_log(failed, succeeds[i]);
        int length = result; //.first;
        if(length > max_length || (length == max_length && succeeds[i]->parsed.size() > max_total) ){
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
    return std::make_pair(max_length, succeeds[max_idx]);
} 

int main (){
    std::ifstream file1("logs/step2.log");
    if (!file1.is_open()) {
        std::cout << "Failed to open logs." << std::endl;
    }

    std::string newLogIndicator = "Method Entry";
    std::string failureIndicator = "BlockManager$ReplicationMonitor";
    
    
    std::string line; 
    // std::unordered_map<int, int> loopStarts; //= {{4, 0}, {1, 0}};
    std::unordered_map <int, int> loopIds;
    
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
            log->loopIds = loopIds;
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
    std::cout << "size " << logs.size() << std::endl;

    std::vector<Log*> succeeds; std::vector<Log*> fails; 
    for(Log* l : logs){
        if(l==nullptr){
            std::cout << "null" << std::endl;
        }
        else if(l->fail){
            fails.push_back(l);
        }else{
            succeeds.push_back(l);
        }
    }
    
    int k = 0;
    for(auto s : fails[k]->to_parse){
        std::cout << s << std::endl;
    }
    fails[k]->parseNextLine();
    
//     std::cout << "//// "; 
//     std::cout << "failed: " << std::endl;
//     for(Log* f : fails){
//         f->printAll();
//     }
//     std::cout << "succeed: " << std::endl;
//     for(Log* s : succeeds){
//         s->printAll();
//     }
//     auto result = logCompare(fails[k], succeeds);
//      //std::cout << max_idx ; // << " L" << fails[2]->getEvent(max_idx)->lineNum << std::endl;
//     int length = result.first;
//      std::cout << "length: " << (length) << ". ";
//      // result.second->printAll();
//      if( (length)==fails[k]->parsed.size() && length==(result.second->parsed.size()) ){
//          std::cout << "no divergence" << std::endl;
//      }else{
//          std::cout << "div at: " ; 
//          if(length==0){
//              if(fails[k]->parsed.size()>1 && fails[k]->getEvent(0)->lineNum==-1){
//                std::cout << "HERE" << std::endl;
//                fails[k]->getEvent(1)->print();
//              }else{
//                  fails[k]->getEvent(0)->print();
//              }
//          }else if(length==1){
//              if(fails[k]->parsed.size()>1 && fails[k]->getEvent(0)->lineNum==-1){
//                  fails[k]->getEvent(1)->print();
//              }else{
//                  fails[k]->getEvent(0)->print();
//              }
//          }else {
//              fails[k]->getEvent(length-1)->print();
//          }
//          std::cout << std::endl;
//      }
//      
//    if(false){
//        std::cout << "failed contexts: " << std::endl;
//         fails[k]->printContexts();
//        std::cout << "succeeds[0] contexts: " << std::endl;
//         succeeds[2]->printContexts();
//        std::cout << "/////////// " << std::endl;
//       for(int i=0; i<fails[k]->parsed.size(); i++){
//           std::cout << fails[k]->getEvent(i)->idx << ": " << fails[k]->getEvent(i)->lineNum << " ";
//       } std::cout << std::endl;
//       for(Event* c : fails[k]->parsed){
//           std::cout << c->idx << ": L" << c->lineNum ;
//           if(c->context != nullptr){std::cout << ": " << c->context->idx << ":L" << c->context->lineNum;}
//           std::cout << std::endl;
//           for(Event* e : fails[k]->contextMap[c->idx]){
//               std::cout << e->idx<< " ";
//           }std::cout << std::endl;
//       } std::cout << std::endl;
//    }
//    return 0;
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

