#!/usr/bin/env python3
from os import path
import subprocess
import ruamel.yaml as yaml

#
# Needs to install the following packages on Ubuntu 18.04 or 20.04
#   sudo apt install -y clang-tools-10 clang-format-10 python3-ruamel.yaml
#

root_dir = path.dirname(path.dirname(path.realpath(__file__))) + '/'
maplefe_dir = root_dir + 'shared/'
output_dir = root_dir + 'output/ast-doc/'
index_yaml = output_dir + 'maplefe.yaml'

license_notice = [
        '/*',
        '* Copyright (C) [2021] Futurewei Technologies, Inc. All rights reverved.',
        '*',
        '* OpenArkFE is licensed under the Mulan PSL v2.',
        '* You can use this software according to the terms and conditions of the Mulan PSL v2.',
        '* You may obtain a copy of Mulan PSL v2 at:',
        '*',
        '*  http://license.coscl.org.cn/MulanPSL2',
        '*',
        '* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER',
        '* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR',
        '* FIT FOR A PARTICULAR PURPOSE.',
        '* See the Mulan PSL v2 for more details.',
        '*/',
        '',
        ]

compile_commands = [
        '[',
        '  { "directory": "' + maplefe_dir + 'src"',
        '    "command":   "clang++ -std=c++17 -DDEBUG -fpermissive -I ' + maplefe_dir + 'include -w -c ast_builder.cpp",',
        '    "file":      "ast_builder.cpp",',
        '    "output":    "' + output_dir + '"',
        '  }',
        ']',
        ]

bash_commands = [
        'cd ' + maplefe_dir + 'src || exit 1',
        'rm -f ' + output_dir + 'yaml.log',
        '[ ast.cpp -nt ' + index_yaml + ' \\',
        '  -o ast_builder.cpp -nt ' + index_yaml + ' \\',
        '  -o ../include/ast.h -nt ' + index_yaml + ' \\',
        '  -o ! -f ' + index_yaml + ' ] || exit 2',
        'clang-doc-10 ast_builder.cpp -p ' + output_dir + ' --format=yaml -output=' + output_dir,
        ]

def exec_command(cmd):
    subprocess.call(cmd, shell=True)

def create(filename, lines):
    with open(filename, "w") as f:
        for line in lines:
            f.write(line + "\n")

def append(filename, lines):
    with open(filename, "a") as f:
        for line in lines:
            f.write(line + "\n")

def finalize(filename, lines):
    append(filename, lines)
    exec_command('clang-format-10 -i --style="{ColumnLimit: 120}" ' + filename)
    print("Generated " + filename)

exec_command('bash -c "mkdir -p ' + output_dir + 'ast2cpp/{src,include}"')
create(output_dir + 'compile_commands.json', compile_commands)
create(output_dir + 'ast.sh', bash_commands)
exec_command('bash ' + output_dir + 'ast.sh')

# Dump all content in a dictionary to ast-doc/yaml.log
def log(dictionary, indent, msg = ""):
    global log_buf
    if indent == 0: log_buf = [msg]
    indstr = " .  " * indent
    for key, value in dictionary.items():
        if key == "USR": continue
        prefix = indstr + key + ' : '
        if isinstance(value, dict):
            log_buf.append(prefix + "{")
            log(value, indent + 1)
            log_buf.append(indstr+ " }")
        elif isinstance(value, list):
            log_buf.append(prefix + "[")
            for elem in value:
                if isinstance(elem, dict):
                    log(elem, indent + 1)
                else:
                    log_buf.append(indstr + "   " + str(elem))
            log_buf.append(indstr+ " ]")
        else:
            log_buf.append(prefix + str(value))
    log_buf.append(indstr + "---")
    if indent == 0:
        append(output_dir + 'yaml.log', log_buf)

# Handle a YAML file with a callback
def handle_yaml(filename, callback, saved_yaml = {}):
    if filename not in saved_yaml:
        print(str(len(saved_yaml) + 1) + ": Processing " + filename + " ...")
        with open(filename) as stream:
            yaml_data = yaml.safe_load(stream)
        saved_yaml[filename] = yaml_data
        log(yaml_data, 0, "YAML file: " + filename)
    else:
        yaml_data = saved_yaml[filename]
    callback(yaml_data)

#################################################################

# Get the pointed-to type, e.g. FunctionNode of "class maplefe::FunctionNode *"
def get_pointed(mtype):
    loc = mtype.find("class maplefe::")
    return mtype[loc + 15:-2] if loc >= 0 and mtype[-6:] == "Node *" else None

# Get enum type, e.g. ImportProperty of "enum maplefe::ImportProperty"
def get_enum_type(mtype):
    loc = mtype.find("maplefe::")
    return mtype[loc + 9:] if loc >= 0 else None

# Get the enum list for given enum name
def get_enum_list(dictionary, enum_name):
    assert dictionary != None
    enums = dictionary["ChildEnums"]
    for e in enums:
        for key, value in e.items():
            if key == "Name" and value == enum_name:
                return e["Members"]
    return []

# Generate functions for enum types, e.g. "const char *getOprId(OprId k);" for enum OprId
def gen_enum_func(dictionary):
    global include_file, src_file, gen_args
    enum_names = [ "TypeId", "SepId", "OprId", "LitId", "AttrId", "NodeKind", "ImportProperty", \
            "OperatorProperty", "DeclProp", "StructProp" ]
    hcode = ['']
    xcode = ['']
    for name in enum_names:
        hcode.append("const char* get" + name + "(" + name + " k);")
        xcode.append("const char* " + gen_args[1] + "::get" + name + "(" + name + " k) {")
        xcode.append("switch(k) {")
        for e in get_enum_list(dictionary, name):
            xcode.append("case " + e + ":")
            xcode.append('return "' + e + '";')
        xcode.append("default: ;  // Unexpected kind")
        xcode.append("}")
        xcode.append('return "UNEXPECTED ' + name + '";')
        xcode.append("}\n")
    append(src_file, xcode)
    append(include_file, hcode)

# Generate code for class node which is derived from TreeNode
def gen_handler_derived(dictionary):
    global include_file, src_file, gen_args
    code = ['']
    node_name = dictionary["Name"];
    assert dictionary["TagType"] == "Class"

    member_functions = {}
    child_functions = dictionary.get("ChildFunctions")
    if child_functions != None:
        for c in child_functions:
            name = c.get("Name")
            member_functions[name] = "R-" + str(c.get("ReturnType").get("Type").get("Name"))

    # gen_func_definition() for the code at the beginning of current function body
    code.append(gen_func_definition(dictionary, node_name))
    members = dictionary.get("Members")
    if members != None:
        declloc = dictionary.get("DefLocation")
        if declloc != None and isinstance(declloc, dict):
            fname = declloc.get("Filename")
            floc = fname.find("shared/")
            code.append("// Declared at " + fname[floc:] + ":" + str(declloc.get("LineNumber")))

        for m in members:
            name = m.get("Name")
            assert name[0:1] == "m"
            otype = m.get("Type").get("Name")
            if otype == "_Bool": otype = "bool"

            if name == "mChildren":
                plural = "Get" + name[1:]
                singular = "GetChild"
            elif name == "mDimensions":
                plural = "GetDims"
                singular = "GetNthDim"
            elif name == "mVars" or name == "mExprs":
                plural = "Get"
                singular = name[1:-1]
            elif name[:3] == "mIs" and otype == "bool":
                plural = name[1:]
                singular = name[1:]
            else:
                plural = "Get" + name[1:]
                singular = "Get" + name[1:-1] if name[-7:] != "Classes" else "Get" + name[1:-2]

            ntype = get_pointed(otype)
            access = m.get("Access")
            accessstr = access if access != None else ""
            if ntype != None:
                prefix = "const_cast<" + ntype + "*>(" if otype[:6] == "const " else ''
                suffix = ")" if prefix != '' else ''
                if member_functions.get(plural) != None:
                    # gen_call_child_node() for child node in current function body
                    code.append(gen_call_child_node(dictionary, name, ntype, prefix + "node->" + plural + "()" + suffix))
                else:
                    # It is an ERROR if no member function for the child node
                    code.append("Error!; // " + gen_call_child_node(dictionary, name, ntype, prefix + "node->" + plural + "()" + suffix))
            elif ((otype == "SmallVector" or otype == "SmallList" or otype == "ExprListNode")
                    and member_functions.get(plural + "Num") != None
                    and (member_functions.get(singular) != None or member_functions.get(singular + "AtIndex") != None)):
                func_name = singular if member_functions.get(singular) != None else singular + "AtIndex"
                rtype = member_functions[func_name][2:]
                if rtype == "_Bool": rtype = "bool"
                # gen_call_children_node() for list or vector of nodes before entering the loop
                code.append(gen_call_children_node(dictionary, name, otype + "<" + rtype + ">", "node->" + plural + "Num()"))
                code.append("for(unsigned i = 0; i < node->" + plural + "Num(); ++i) {")
                ntype = get_pointed(rtype)
                if ntype != None:
                    prefix = "const_cast<" + ntype + "*>(" if rtype[:6] == "const " else ''
                    suffix = ")" if prefix != '' else ''
                    # gen_call_nth_subchild_node() for the nth subchild node in the loop for the list or vector
                    code.append(gen_call_nth_subchild_node(dictionary, name, ntype, prefix + "node->" + func_name + "(i)" + suffix))
                else:
                    # gen_call_nth_subchild_value() for the nth subchild value in the loop for the list or vector
                    code.append(gen_call_nth_subchild_value(dictionary, name, rtype, "node->" + func_name + "(i)"))
                code.append("}")
                code.append(gen_call_children_node_end(dictionary, name, otype + "<" + rtype + ">", "node->" + plural + "Num()"))
            else:
                if member_functions.get(plural) != None:
                    # gen_call_child_value() for child value in current function body
                    code.append(gen_call_child_value(dictionary, name, otype, "node->" + plural + "()"))
                else:
                    # It is an ERROR if no member function for the child value
                    code.append("Error!; // " + gen_call_child_value(dictionary, name, otype, "node->" + plural + "()"))

    # gen_func_definition_end() for the code at the end of current function body
    code.append(gen_func_definition_end(dictionary, node_name))
    append(src_file, code)

    code = []
    code.append(gen_func_declaration(dictionary, node_name))
    append(include_file, code)

# Generate handler for TreeNode
def gen_handler_tree_node(dictionary):
    global include_file, src_file, gen_args
    code = ['']
    code.append(gen_func_declaration(dictionary, "TreeNode"))
    append(include_file, code)

    code = ['']
    code.append(gen_func_definition(dictionary, "TreeNode"))

    code.append("switch(node->GetKind()) {")
    for flag in get_enum_list(dictionary, "NodeKind"):
        code.append("case " + flag + ":");
        node_name = flag[3:] + "Node"
        filename = output_dir + 'maplefe/' + node_name + '.yaml'
        if path.exists(filename):
            # gen_call_child_node() for visiting child node
            code.append(gen_call_child_node(dictionary, "", node_name, "static_cast<" + node_name + "*>(node)"))
        elif node_name == "NullNode":
            code.append("// Ignore NullNode")
        else:
            # it is an ERROR if the node kind is out of range
            code.append("Error!!! // " + gen_call_child_node(dictionary, "", node_name, "static_cast<" + node_name + "*>(node)"))
        code.append("break;");
    code.append("default: ;  // Unexpected kind")
    code.append("}")
    code.append("return node;")
    code.append("}")
    append(src_file, code)

# Handle each node which has TreeNode as its base
def gen_handler_node(dictionary):
    base = dictionary.get("Bases")
    if base != None:
        basename = base[0].get("Name")
        if basename == "TreeNode":
            gen_handler_derived(dictionary)

# Check each child records
def gen_handler(dictionary):
    child_records = dictionary["ChildRecords"]
    for child in child_records:
        value = child["Name"]
        filename = output_dir + 'maplefe/' + value + '.yaml'
        if path.exists(filename):
            handle_yaml(filename, gen_handler_node)
    # Generate handler for TreeNode
    gen_handler_tree_node(dictionary)

###################################################################################################

# Initialize/finalize include_file and src_file with gen_args
Initialization = 1
Finalization = 2
def handle_src_include_files(phase):
    global include_file, src_file, gen_args
    include_file = output_dir + "ast2cpp/include/" + gen_args[0] + ".h"
    src_file = output_dir + "ast2cpp/src/" + gen_args[0] + ".cpp"
    include_start = [
            '#ifndef __' + gen_args[1].upper() + '_HEADER__',
            '#define __' + gen_args[1].upper() + '_HEADER__',
            '',
            '#include "ast_module.h"',
            '#include "ast.h"',
            '#include "ast_type.h"',
            '#include "ast_attr.h"',
            '',
            'namespace maplefe {',
            '',
            'class ' + gen_args[1] + ' {',
            'public:',
            ]
    include_end = [
            '};',
            '',
            '}',
            '#endif',
            ]
    src_start = [
            '#include "' + gen_args[0] + '.h"',
            '',
            'namespace maplefe {',
            '',
            ]
    src_end = [
            '',
            '}',
            ]
    if phase == Initialization:
        create(include_file, license_notice + include_start)
        create(src_file, license_notice + src_start)
    elif phase == Finalization:
        finalize(include_file, include_end)
        finalize(src_file, src_end)

###################################################################################################

def gen_func_declaration(dictionary, node_name):
    return node_name + "* " + gen_args[2] + node_name + "(" + node_name + "* node);"

def gen_func_definition(dictionary, node_name):
    return node_name + "* " + gen_args[1] + "::" + gen_args[2] + node_name + "(" + node_name + "* node) {"

def gen_call_child_node(dictionary, field_name, node_type, accessor):
    return gen_args[2] + node_type + "(" + accessor + ");"

def gen_call_child_value(dictionary, field_name, val_type, accessor):
    return "// Value: " + gen_args[2] + " " + val_type + "(" + accessor + ");"

def gen_call_children_node(dictionary, field_name, node_type, accessor):
    return "// field: " + field_name + ", type: " + node_type

def gen_call_children_node_end(dictionary, field_name, node_type, accessor):
    return ""

def gen_call_nth_subchild_node(dictionary, field_name, node_type, accessor):
    return gen_args[2] + node_type + "(" + accessor + ");"

def gen_call_nth_subchild_value(dictionary, field_name, val_type, accessor):
    return "// Value: " + gen_args[2] + " " + val_type + "(" + accessor + ");"

def gen_func_definition_end(dictionary, node_name):
    return "return node;\n}"

#
# Generate a2c_handler.h and a2c_handler.cpp
#
gen_args = [
        "a2c_handler", # filename
        "A2C_Handler", # Class name
        "Handle",      # Prefix of function name
        ]

# Example to extract code pieces starting from ast-doc/maplefe/index.yaml
if False:
    handle_src_include_files(Initialization)
    handle_yaml(index_yaml, gen_handler)
    handle_src_include_files(Finalization)

################################################################################

def gen_func_definition(dictionary, node_name):
    str = node_name + "* " + gen_args[1] + "::" + gen_args[2] + node_name + "(" + node_name + "* node) {" \
            + '\nif(node == nullptr) {\n' \
                 + 'dump("  ' + node_name + ': null");\n' \
                 + 'return node;\n' \
              + '}'
    if node_name != "TreeNode":
        str += '\ndump("  ' + node_name + ' {");\n' \
                + 'indent += 4;\n' \
                + 'base(node);'
    return str

def gen_call_child_node(dictionary, field_name, node_type, accessor):
    str = 'dump("' + field_name + ': ' + node_type + '*");\n' if field_name != "" else ""
    return str + gen_args[2] + node_type + "(" + accessor + ");"

def gen_call_child_value(dictionary, field_name, val_type, accessor):
    str = 'dump(std::string("' + field_name + ': '
    e = get_enum_type(val_type)
    if e != None:
        str += e + ': ") + get' + e + '(' + accessor + '));\n'
    elif val_type == "LitData":
        str += 'LitData: LitId, ") + getLitId(' + accessor + '.mType) + ", " + getLitData(' + accessor + '));\n'
    elif val_type == "bool":
        str += val_type + ', ") + (' + accessor + ' ? "true" : "false"));\n'
    elif val_type == "unsigned int":
        str += val_type + ', " + std::to_string(' + accessor + '));\n'
    else:
        str += val_type + ', ") + "value");\n'
    return str

def gen_call_children_node(dictionary, field_name, node_type, accessor):
    str = 'dump("' + field_name + ': ' + node_type + ', size = " + std::to_string(' + accessor + ') + " [");\n'
    return str + 'indent += 2;'

def gen_call_children_node_end(dictionary, field_name, node_type, accessor):
    return 'indent -= 2;\ndump(" ]");'

def gen_call_nth_subchild_node(dictionary, field_name, node_type, accessor):
    str = 'dump(std::to_string(i + 1) + ": ' + node_type + '*");\n'
    return str + gen_args[2] + node_type + "(" + accessor + ");"

def gen_call_nth_subchild_value(dictionary, field_name, val_type, accessor):
    str = 'dump(std::to_string(i) + ". '
    e = get_enum_type(val_type)
    if e != None:
        str += e + ': " + get' + e + '(' + accessor + '));\n'
    elif val_type == "LitData":
        str += 'LitData: LitId, " + getLitId(' + accessor + '.mType) + ", " + getLitData(' + accessor + '));\n'
    elif val_type == "bool":
        str += val_type + ', ") + (' + accessor + ' ? "true" : "false"));\n'
    elif val_type == "unsigned int":
        str += val_type + ', " + std::to_string(' + accessor + '));\n'
    else:
        str = 'dump(std::string("' + field_name + ': ' + val_type + ', ") ' + ' + "value");\n'
    return str

def gen_func_definition_end(dictionary, node_name):
    return 'indent -= 4;\ndump("  }");\nreturn node;\n}'

#
# Generate a2c_astdump.h and a2c_ast.cpp
#
gen_args = [
        "a2c_astdump", # filename
        "A2C_AstDump", # Class name
        "AstDump",     # Prefix of function name
        ]

astdump_init = [
        'private:',
        'int indent;',
        '',
        'public:',
        gen_args[1] + '() : indent(0) {}',
        '',
        'void dump(TreeNode* node) {',
        'AstDumpTreeNode(node);',
        '}',
        '',
        'private:',
        'void dump(const std::string& msg) {',
        'std::cout << std::string(indent, \' \') << msg << std::endl;',
        '}',
        '',
        'void base(TreeNode* node) {',
        'dump(std::string(". mKind: NodeKind, ") + getNodeKind(node->GetKind()));',
        'const char* name = node->GetName();',
        'if(name)',
        'dump(std::string(". mName: const char*, \\"") + node->GetName() + "\\"");',
        'TreeNode* label = node->GetLabel();',
        'if(label) {',
        'dump(". mLabel: TreeNode*: ");',
        'AstDumpTreeNode(label);'
        '}',
        '}',
        '',
        'std::string getLitData(LitData lit) {',
        'std::string str = getLitId(lit.mType);',
        'switch (lit.mType) {',
        'case LT_IntegerLiteral:',
        'return std::to_string(lit.mData.mInt);',
        'case LT_FPLiteral:',
        'return std::to_string(lit.mData.mFloat);',
        'case LT_DoubleLiteral:',
        'return std::to_string(lit.mData.mDouble);',
        'case LT_BooleanLiteral:',
        'return std::to_string(lit.mData.mBool);',
        'case LT_CharacterLiteral:',
        'return std::string(1, lit.mData.mChar.mData.mChar); // TODO: Unicode support',
        'case LT_StringLiteral:',
        'return std::string(lit.mData.mStr);',
        'case LT_NullLiteral:',
        'return std::string("null");',
        'case LT_ThisLiteral:',
        'return std::string("this");',
        'case LT_SuperLiteral:',
        'return std::string("super");',
        'case LT_NA:',
        'return std::string("NA");',
        'default:;',
        '}',
        '}',
        '',
        ]

handle_src_include_files(Initialization)
append(include_file, astdump_init)
handle_yaml(index_yaml, gen_handler)
handle_yaml(index_yaml, gen_enum_func)
handle_src_include_files(Finalization)

