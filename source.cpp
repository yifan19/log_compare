#include "source.hpp"

Variable::Variable(int line, std::string n) : lineDef(line), name(n) {}

/// 
Invocation::Invocation(std::string n) : name(n) {}

/// 
bool is_var_type(std::string& name);
Node* root;
std::vector<Node*> nodes;
//
bool parseJava(std::string path){
    std::ifstream file(path); root = nullptr;
    int lineIdx = 1;
    // make something up
    if (!file.is_open()) {
        std::cout << "Failed to open the file." << std::endl;
        return false; '[-p0'
    }
    std::string line;
    root = nullptr; Node* parent = root;
    while (std::getline(file, line)) {
        if (line.find("// *ignore") != std::string::npos) {
            nodes.push_back(nullptr); // ignore this line
            lineIdx++; continue;
        }
        size_t comment = line.find("//");
        if (comment != std::string::npos) {
            line = line.substr(0, comment);
        }
        std::stringstream ss(line);  // tokenization
        // std::vector<std::string> tokens(std::istream_iterator<std::string>{ss}, std::istream_iterator<std::string>());
        std::string token;
        ss >> token; Node* e = nullptr;
        if(token == "if" || token ==  "while" || token == "for"){
            e = new CE(lineIdx);
            std::getline(ss, line, ')'); // if something is after the () it will be gone DON'T do that
            line = line.substr(line.find('(')); // extract thing in the bracket
            e->parent = parent; // set parent of this 
            parent = e; // the following block's parent is this event

        }else if(is_var_type(token)){
            std::string name;
            ss >> name; auto it = variables.find(name);
            // it should not be java-var type, but I'll assume the thing compiles
            if(it!=variables.end()){
                e = new LE(lineIdx);
                LE->
            }
        }else{

        }

        lineIdx ++;
    }

    file.close();  
    return true;
}

bool is_var_type(std::string& name){
    return (java_var_types.find(name) != java_var_types.end());
}

