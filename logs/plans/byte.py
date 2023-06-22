#!/usr/bin/python3

import re
import json

function_signatures = {}
with open('function_signatures.json', 'r') as f:
    function_signatures = json.load(f)

print(function_signatures)

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
        result = {
            "type": "after_print",
            "id": id_match.group(1),
	        "function": after_print_match.group(2),
	        "class": after_print_match.group(1).replace("/", "."),
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
        result = {
            "type": "before_print",
            "id": id_match.group(1),
            "function": before_print_match.group(2),
            "class": before_print_match.group(1).replace("/", "."),
            "line": before_print_match.group(3),
            "loop": loop
        }
        if argument_match:
            result["argument"] = argument_match.group(2)
        return result
    elif branch_id_match and branch_for_match:
        return {
            "type": "branch",
            "branch_id": branch_id_match.group(1),
            "for": branch_for_match.group(1)
        }

    elif constant_value_match:
        return {
            "type": "constant_value",
            "id": id_match.group(1),
            "value": constant_value_match.group(1)
        }

    elif accessing_field_match:
        return {
            "type": "accessing_field",
            "id": id_match.group(1),
            "class": accessing_field_match.group(1)
        }

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
    else:
        return None
    condition = "true"
    print_statement = f'traceln("[BM][" + Thread.currentThread().getName() + "]ID={rule_id},"'
    if loop:
        print_statement += ' + "loop=1,"'
    if "argument" in instruction:
        print_statement += f' + (${instruction["argument"]})'
    print_statement += ')'

    rule = f'''RULE ID {rule_id}
CLASS {class_name}
METHOD {function}
COMPILE
AT {position}
IF {condition}
DO {print_statement}
ENDRULE
'''
    print(rule)
    return rule

with open('instructions.txt', 'r') as f:
    content = f.read()
    match = re.search(r'(Branch ID: )?(.*?)(?=What is the ID to continue|$)', content, re.DOTALL)
    if match:
        content = match.group(2)
    print(content)
    instructions = content.split("\n\n")
    parsed = [parse_instruction(instruction) for instruction in instructions]
    for p in parsed:
        print(p)
        print("///")
    for p in parsed:
        to_byteman_rule(p)

with open('function_signatures.json', 'w') as f:
    json.dump(function_signatures, f)
