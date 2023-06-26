#include <iostream>
#include <sstream> 
#include <fstream>
#include <string> 
#include <cstdio>

#include <vector>
#include <map>
#include <unordered_map>
#include <regex>

#include "Event.hpp"
#include "Log.hpp"
#include "globals.hpp"
#define DIV 0
#define TRACE 1
#define ARG 2

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
                                                                                                                        
int main (int argc, char *argv[]){    ////////////////////////////////////////////////////////////////////
    std::string file_path = "logs/step1a2.log";
    std::string base_path = "/home/ubuntu/hadoop/hadoop-hdfs-project/hadoop-hdfs/src/main/java/";
    int what_to_do = DIV;
    if(argc>=2){
        file_path = argv[1];
    } //"";
    if(argc>=3){
        std::stringstream ss (argv[2]);
        ss >> what_to_do;
        std::cout << "to do: " << what_to_do << std::endl;
    }
    std::ifstream file1(file_path);
    std::cout << "file path: " << file_path << std::endl;
    if (!file1.is_open()) {
        std::cout << "Failed to open logs." << std::endl;
        return 1;
    }

    
    std::string failureIndicator = "BlockManager$ReplicationMonitor";
    std::string newLogIndicator = "Method Entry";
    std::string arg_value = "-1";
    if(argc>=4){
        std::cout << "Indicator: " << failureIndicator << std::endl;
    } //"";
    if(argc>=5){
        newLogIndicator = argv[4];
    }
    
    std::string line; 
    // std::unordered_map<int, int> loopStarts; //= {{4, 0}, {1, 0}};
    // std::unordered_map <int, int> loopIds = {{4, 1},{3, 1},{2, 1},{1, 2}, {0, 2}};
    // std::unordered_map <int, int> loopStartIds = {{4, 1},{1, 2}}; std::unordered_map <int, int> parentLoop = {{1,-1}, {2,1}};
    if(what_to_do == TRACE){
        std::cout << "______" << std::endl;
        std::cout << "finding callers of function " << failureIndicator << ": " << std::endl;
        bool next = false; bool found = false;
        std::string::size_type temp_id;

        while(std::getline(file1, line)){
            temp_id = line.find("Stack Trace");
            if(temp_id == std::string::npos){
                next = false;
                continue;
            }
            // std::cout << "line: " << line << std::endl;
            if(next){
                line = line.substr(5 );
                temp_id = line.find("]");
                if(temp_id != std::string::npos){
                    line = line.substr(0, temp_id);
                }
                std::cout << line << std::endl;
                std::regex pattern(R"((.*)\.(.*)\((.*):(\d+)\))");
                std::smatch match;
                if (std::regex_search(line, match, pattern) && match.size() > 1) {
                    std::string src_path = match.str(1);  // Get the class name
                    std::string method_name = match.str(2);  // Get the method name
                    std::string file_name = match.str(3);  // Get the file name
                    int lineNum = std::stoi(match.str(4));  // Get the line number
                    std::replace(src_path.begin(), src_path.end(), '.', '/');
                    src_path = base_path + src_path + ".java";
                    // std::cout << "file path: " << src_path << std::endl;
                    // std::cout << "line: " << lineNum << std::endl;
                    
                    // std::ifstream file(src_path);
                    // std::string src_line;
                    // int numRead = 0;
                    // if (file.is_open()) {
                    //     while (std::getline(file, src_line)) {
                    //         numRead++;
                    //         if (numRead == lineNum) {
                    //             std::cout << src_line << std::endl;
                    //             break;
                    //         }
                    //     }
                    //     file.close();
                    // } else {
                    //     std::cout << "Unable to open file";
                    // }
                } else {
                    std::cerr << "No match\n";
                }

                next = false; found = true;
            }

            temp_id = line.find(failureIndicator);
            if(temp_id != std::string::npos){
                next = true;
            }else{
                next = false;
            }
        }
        if(!found){
            std::cout << "Did not find caller of " << failureIndicator << std::endl;
        }
        return 0;
    }
    else if(what_to_do == ARG){ ///////////////////// VARIABLE //////////////////////////////////
        std::cout << "searching for argument value " << arg_value << std::endl;
        bool found = false;
        std::string arg_value = ""; 
        std::string::size_type temp_id;
        Log* log = new Log(); int idx = 0;
        while(std::getline(file1, line)){
            log->to_parse.push_back(line);
            log->parseNextLine();
            temp_id = line.find("Field Member");
            if(temp_id != std::string::npos){
                temp_id = line.find(failureIndicator);
                if(temp_id != std::string::npos){
                    Event* e = log->getEvent(idx);
                    if(e != nullptr) {
                        arg_value = e->value;
                        e->loopId = 1;
                        found = true;
                    }
                }
                
            }
            idx++;
        }
        if(!found){
            std::cout << "______" << std::endl;
            std::cout << "did not find value as target" << std::endl;
            return 0;
        }
        found = false;
        std::cout << "______" << std::endl;
        for(Event* e : log->parsed){
            if(e != nullptr && e->value==arg_value && e->loopId!=1){
                std::cout << "first log with value " << arg_value << ": " << std::endl;
                e->print(); 
                std::cout << std::endl;
                found = true;
                break;
            }
        }
        if(!found){
            std::cout << "Did not find logs with argument value " << arg_value << std::endl;
        }
        return 0;
    }
    
    std::vector<Log*> logs; //
    // std::cout << "HERE" << std::endl;
    std::unordered_map<int, Log*> threads;
    int num_fails = 0;
    int num_threads = 0; bool fail_encountered = false;
    while(std::getline(file1, line)){
        bool newLog = false; int thread = -1; bool fail = false;
        std::cout << line << std::endl; 
        std::string::size_type temp_id = line.find("Stack Trace");
        if(temp_id != std::string::npos){
            continue;
        }
        temp_id = line.find("IPC Server handler ");
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
        std::cout << "did not find log for failure runs" << std::endl;
        return 1;
    }
    int k = 0;
//    std::cout << "failed run: " << std::endl;
//    fails[k]->printContexts();
//    fails[k]->printContexMaps();
//    std::cout << "good run: " << std::endl;
//    succeeds[1]->printContexts();
    // fails[k]->printLoops();
    std::cout << std::endl;
    if(what_to_do == DIV){  ////////////////// DIVERGENCE //////////////////////////////////////////
        std::cout << "comparing logs" << std::endl;
        auto result = logCompare(fails[k], succeeds);
        int length = result.second.size();
        std::cout << "length: " << (length) << ". ";
        std::cout << "______" << std::endl;
            
        if( length==fails[k]->parsed.size() && length==result.first->parsed.size() ){
            std::cout << "prefix: " << std::endl;
            for(int i=0; i<result.second.size(); i++){
                std::cout << result.second[i].idx << ":ID=" << result.second[i].lineNum << " ";
            }std::cout << std::endl << std::endl;
              std::cout << "No divergence" << std::endl;
              
        }else{

            std::cout << "Diverge at: " ; 
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

