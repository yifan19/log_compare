#!/usr/bin/python3

import re
import json
import time
import subprocess
import os
import glob

input_path = '/home/ubuntu/instructions.txt'
bash_path = '/home/ubuntu/hadoop/run_c.sh'
log_path = '/home/ubuntu/log/log_compare/logs'
py_path = '/home/ubuntu/log/log_compare/logs/code'
btm_path = '/home/ubuntu/log/log_compare/logs/plans'
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
        print_statement = 'traceln("[BM][" + Thread.currentThread().getStackTrace()[5] + "]");\n   traceln("[BM][" + Thread.currentThread().getStackTrace()[6] + "]")'

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

def process_command(content):
    match = re.search(r'(Branch ID: )?(.*?)(?=What is the ID to continue|$)', content, re.DOTALL)
    if match:
        content = match.group(2)
    print(content)
    branches = content.replace("Branch ID", "[!!]Branch ID").split("[!!]")
    print("\n\n////////////////////////\n\n")
    i = 0
    branch_type = 
    for b in branches:
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
            if p is None: continue
            parsed.append(p)
            # print(p)
            methods.add(p["function"])

        rules = []
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

def run():
    start_pos = 0
    while True:
        result, start_pos = find_between(input_path, start_pos)
        if result is not None:
            content = ''.join(result)
            print(content)
            process_command(content)
            os.chdir("/home/ubuntu/hadoop/")
            subprocess.run(['bash', bash_path])
            print("bash done")
            os.chdir('/home/ubuntu/log/log_compare/')
            log_files = glob.glob(os.path.join(log_path, "current_b*"))
            for (root, dirs, files) in os.walk(log_path):
                print(files)
            for log in log_files:
                print(log)
                # with open(log, 'r') as f:
                #    lines = f.read()
                #    print(lines)
                result = subprocess.run([compare, log], capture_output=True, text=True)
                print("Output:", result.stdout)
                print( result.stderr)
                # os.remove(log)
            btm_files = glob.glob(os.path.join(btm_path, "current_b*"))
            for btm in btm_files:
                print("remove", btm)
                # os.remove(btm)
            os.chdir("/home/ubuntu/log/log_compare/logs/code")
            print('\nWaiting for instructions\n')
            # break
        time.sleep(2)  

run()

#process_command('instructions.txt')

with open('function_signatures.json', 'w') as f:
    json.dump(function_signatures, f)
with open('function_classes.json', 'w') as f:
    json.dump(function_classes, f)

