#!/usr/bin/python3

import re
import json
import time
import subprocess
import os
import glob

input_path = '/home/ubuntu/instructions.txt'
bash_path = '/home/ubuntu/log/log_compare/python/run.sh'
log_path = '/home/ubuntu/log/log_compare/python/logs'
py_path = '/home/ubuntu/log/log_compare/python'
btm_path = '/home/ubuntu/log/log_compare/python/plans'
compare = '/home/ubuntu/log/log_compare/compare'

function_signatures = {}
with open('function_signatures.json', 'r') as f:
    function_signatures = json.load(f)
function_classes = {}
with open('function_classes.json', 'r') as c:
    function_classes = json.load(c)

def parse_instruction(instruction):
    id_match = re.search(r"ID: (\d+)", instruction)
    branch_id_match = re.search(r"Branch ID: (\d+)", instruction)
    branch_for_match = re.search(r"For: (.*)", instruction)
    constant_value_match = re.search(r"Constant value: (.*)", instruction)
    accessing_field_match = re.search(r"Class type is (.*)\nAccessing", instruction)
    loop = ("Instruction is in a loop" in instruction)
    argument_match = re.search(r"(\n)(.+)(\nPrint argument)", instruction)

    if "At function entry" in instruction:
        function_match = re.search(r"At function entry (.*)\nIn file: (.+)", instruction)
        if not id_match or not function_match:
            return None
        function_name = function_match.group(1).partition("(")[0]
        class_name = function_match.group(2).replace("/", ".")
        if class_name.endswith(".java"):
            class_name = class_name[:-5]
        result = {
            "type": "function_entry",
            "id": id_match.group(1),
            "function": function_name,
            "class": class_name,
            "loop": loop
        }
        if argument_match:
            result["argument"] = argument_match.group(2)
        return result
    elif "Print after the following source location" in instruction:
        after_print_match = re.search(r"Print after the following source location \n(.+) (.+):(\d+)", instruction)
        if not id_match or not after_print_match:
            return None
        class_name = after_print_match.group(1).replace("/", ".")
        if class_name.endswith(".java"):
            class_name = class_name[:-5]
        result = {
            "type": "after_print",
            "id": id_match.group(1),
	    "function": after_print_match.group(2),
            "class": class_name,
            "line": after_print_match.group(3),
            "loop": loop
        }
        if argument_match:
            result["argument"] = argument_match.group(2)
        return result
        
    elif "Print before the following source location" in instruction:
        before_print_match = re.search(r"Print before the following source location \n(.+) (.+):(\d+)", instruction)
        if not id_match or not before_print_match:
            return None
        class_name = before_print_match.group(1).replace("/", ".")
        if class_name.endswith(".java"):
            class_name = class_name[:-5]
        result = {
            "type": "before_print",
            "id": id_match.group(1),
            "function": before_print_match.group(2),
            "class": class_name,
            "line": before_print_match.group(3),
            "loop": loop
        }
        if argument_match:
            result["argument"] = argument_match.group(2)
        return result
        
    elif "Print or use an existing stacktrace" in instruction:
        function_match = re.search(r"caller function of(.+)_(.+)_(.+)", instruction)
        result = {
            "id": "stack trace",
            "type": "stack_trace",
            "function": function_match.group(2),
            "class": function_match.group(1),
            "suffix": function_match.group(3),
            "loop": False
        }
        return result
    elif "Accessing field member" in instruction:
        class_match = re.search(r"Accessing field member of a class(.+). So print variable", instruction)
        result = {
            "id": "access field",
            "type": "print_var",
            "class": class_match.group(1),
            "loop": False
        }
        return result

    return None

def to_byteman_rule(instruction):
    rule_id = instruction["id"]
    function_name = instruction["function"]
    function = function_signatures[function_name]
    if not function:
        print("function not found:", function_name)
        return None
    class_name = instruction["class"]
    loop = instruction["loop"]
    rule_type = instruction["type"]

    if rule_type == "function_entry":
        position = "ENTRY"
    elif rule_type == "before_print":
        position = "LINE " + instruction["line"]
    elif rule_type == "after_print":
        position = "LINE " + instruction["line"]
    elif rule_type == "stack_trace":
        position = "ENTRY"
    else:
        return None
    condition = "true"
    print_statement = f'traceln("[BM][" + Thread.currentThread().getName() + "]ID={rule_id},"'
    if loop:
        print_statement += ' + "loop=1,"'
    if "argument" in instruction:
        print_statement += f' + (${instruction["argument"]})'
    print_statement += ')'

    if rule_type == "stack_trace":
        print_statement = 'traceln("[BM][" + Thread.currentThread().getStackTrace()[5] + "][Stack Trace]");\n   traceln("[BM][" + Thread.currentThread().getStackTrace()[6] + "][Stack Trace]")'

    rule = f'''RULE ID {rule_id}
CLASS {class_name}
METHOD {function}
COMPILE
AT {position}
IF {condition}
DO {print_statement}
ENDRULE
'''
    return rule
def print_var(instructions, p0):
    print("HERE")
    
def process_command(content):
    match = re.search(r'(Branch ID: )?(.*?)(?=What is the ID to continue|$)', content, re.DOTALL)
    if match:
        content = match.group(2)
    print(content)
    branches = content.replace("Branch ID", "[!!]Branch ID").split("[!!]")
    print("\n\n////////////////////////\n\n")
    i = 0
    
    b_types = {}
    b_rules = {}
    for b in branches:
        branch_type = 0 # DIV
        print("THERE")
        instructions = b.split("\n\n")
        branch_match = re.search(r"Branch ID: (\d+)", instructions[0])
        print(b)
        print("!!!!!!!")
        if not branch_match:
            if i != 0:
                continue
            else:
                branch_id = "0"
        else:
            branch_id = branch_match.group(1)
        i += 1
        print("Branch ID is", branch_id, "///////")
        parsed = []
        methods = set()
        for instruction in instructions:
            instruction = instruction.strip("\n")
            # if not instruction.startswith("ID: "):
                #
            p = parse_instruction(instruction)
            if p is None:
                continue
            if p["type"] == "print_var":
                branch_type = 0
                print_var(instructions, p)
                continue
            parsed.append(p) # do not append if is print_var
            methods.add(p["function"])
            if p["type"] == "stack_trace":
                branch_type = 1
                break
            
            # print(p)

        rules = []
        if branch_type == 0:
            for method in methods:
                if method not in function_signatures:
                    print("signature not found: ", method)
                    continue
                function = function_signatures[method]
                rule = f'''RULE entering {method}
CLASS {function_classes[function]}
METHOD {function}
COMPILE
AT ENTRY
IF true
DO traceStack("[BM][" + Thread.currentThread().getName() + "][Method Entry]");
ENDRULE
'''
                rules.append(rule)
                print(rule)
        for p in parsed:
            rule = to_byteman_rule(p)
            rules.append(rule)
            print(rule)
        file_name = btm_path + '/current_b' + branch_id + '.btm'
        print("write to:", file_name)
        with open(file_name, 'w') as f:
            for rule in rules:
                print(rule)
                f.write(rule)
                f.write('\n')
        b_types[branch_id] = branch_type
        b_rules[branch_id] = parsed
    return b_rules, b_types


def find_between(file_path, start_pos):
    with open(file_path, 'r') as f:
        f.seek(start_pos)
        lines = f.readlines()
        new_start_pos = f.tell()
        for i in reversed(range(len(lines))):
            if 'What is the ID to continue' in lines[i]:
                for j in reversed(range(i)):
                    if 'instrumentation required' in lines[j]:
                        return lines[j:i+1], new_start_pos
                break
    return None, start_pos

def parse_results(results):
    for id in results.keys():
        print("Branch ID =", id)
        # print(results[id].stdout)
        out = results[id].stdout.partition("______")[2]
        print(out, '\n')

def run():
    start_pos = 0
    print('\nWaiting for instructions\n')
    while True:
        found, start_pos = find_between(input_path, start_pos)
        if found is not None:
            content = ''.join(found)
            print(content)
            for root, dirs, files in os.walk(btm_path, topdown=False):
                for file in files:
                    print("removing", file)
                    os.remove(os.path.join(root, file))
            b_rules, b_types = process_command(content) # generate btm
            os.chdir("/home/ubuntu/hadoop/")
            for root, dirs, files in os.walk(log_path, topdown=False):
                for file in files:
                    print("removing", file)
                    os.remove(os.path.join(root, file))
            for b in b_rules.keys():
                subprocess.run(['bash', bash_path], input=b, text=True)
                print("bash done:", b)
            os.chdir('/home/ubuntu/log/log_compare/')
            log_files = glob.glob(os.path.join(log_path, "current_b*"))
            results = {}
            for log in log_files:
                print(log)
                # with open(log, 'r') as f:
                #    lines = f.read()
                #    print(lines)
                branch_id = re.search(r"current_b(\d+).log", log).group(1)
                print("///////////////////////////////////////////////////")
                print("B id:", branch_id)
                branch_type = b_types[branch_id]
                parsed = b_rules[branch_id]

                # print("branch types:", b_types)
                # print("rules ", b_rules)
                # print("parsed", parsed)
                
                if branch_type == 0:
                    result = subprocess.run([compare, log, '0'], capture_output=True, text=True)
                    results[branch_id] = result
                elif branch_type == 1:
                    function = ''
                    for p in parsed:
                        print("p: " , p)
                        print(p["type"])
                        if p["type"] == 'stack_trace':
                            function = p["function"]
                            result = subprocess.run([compare, log, '1', function], capture_output=True, text=True)
                            results[branch_id] = result
                            break
                    if function == '':
                        print("panick")
                print("Output:", result.stdout)
                print( result.stderr)
                # os.remove(log)
            btm_files = glob.glob(os.path.join(btm_path, "current_b*"))
            for btm in btm_files:
                print("remove", btm)
                # os.remove(btm)
            print("------")
            parse_results(results)
            print('\nWaiting for instructions\n')
            # break
        time.sleep(2)  

run()

#process_command('instructions.txt')

with open('function_signatures.json', 'w') as f:
    json.dump(function_signatures, f)
with open('function_classes.json', 'w') as f:
    json.dump(function_classes, f)

