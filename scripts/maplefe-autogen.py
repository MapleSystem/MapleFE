#!/usr/bin/env python3
from os import path, environ
import subprocess
import ruamel.yaml as yaml

#
# Needs to install the following packages on Ubuntu 18.04 or 20.04
#   sudo apt install -y clang-tools-10 clang-format-10 python3-ruamel.yaml
#

root_dir = path.dirname(path.dirname(path.realpath(__file__))) + '/'
builddir = environ.get('BUILDDIR')
output_dir = builddir + '/ast_doc/' if builddir != None else root_dir + "output/typescript/ast_doc/"
maplefe_dir = root_dir + 'shared/'
# initial_yaml = output_dir + 'maplefe/index.yaml' # For higher version of clang-doc
initial_yaml = output_dir + 'maplefe.yaml'         # For version 10
treenode_yaml = output_dir + 'maplefe/TreeNode.yaml'

license_notice = [
"""/*
* Copyright (C) [2021] Futurewei Technologies, Inc. All rights reverved.
*
* OpenArkFE is licensed under the Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*
*  http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
* FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v2 for more details.
*/

// Generated by maplefe-autogen.py
"""
] # license_notice

compile_commands = [
"""
[
  {{ "directory": "{maplefe_dir}src"
    "command":   "clang++ -std=c++17 -DDEBUG -fpermissive -I {maplefe_dir}include -w -c ast_builder.cpp",
    "file":      "ast_builder.cpp",
    "output":    "{output_dir}"
  }}
]
""".format(maplefe_dir=maplefe_dir, output_dir=output_dir)
] # compile_commands

bash_commands = [
"""
cd {maplefe_dir}src || exit 1
rm -f {output_dir}yaml.log
clang-doc-10 ast_builder.cpp -p {output_dir} --format=yaml -output={output_dir}
""".format(maplefe_dir=maplefe_dir, output_dir=output_dir)
] # bash_commands

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

exec_command('bash -c "mkdir -p ' + output_dir + 'shared"')
create(output_dir + 'compile_commands.json', compile_commands)
create(output_dir + 'ast.sh', bash_commands)
exec_command('bash ' + output_dir + 'ast.sh')

# Dump all content in a dictionary to ast_doc/yaml.log
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

# Generate functions for enum types, e.g. "const char *GetEnumOprId(OprId k);" for enum OprId
def gen_enum_func(dictionary):
    global include_file, src_file, gen_args
    hcode = ['']
    xcode = ['']
    for each in dictionary["ChildEnums"]:
        name = each["Name"]
        hcode.append("static const char* GetEnum" + name + "(" + name + " k);")
        xcode.append("const char* " + gen_args[1] + "::GetEnum" + name + "(" + name + " k) {")
        xcode.append("switch(k) {")
        for e in get_enum_list(dictionary, name):
            xcode.append("case " + e + ":")
            xcode.append('return "' + e + '";')
        xcode.append('default: MASSERT(0 && "Unexpected enumerator");')
        xcode.append("}")
        xcode.append('return "UNEXPECTED ' + name + '";')
        xcode.append("}\n")
    append(src_file, xcode)
    append(include_file, hcode)

# Generate code for class node which is derived from TreeNode
def gen_handler_ast_node(dictionary):
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
        if gen_func_decl_location() and declloc != None and isinstance(declloc, dict):
            fname = declloc.get("Filename")
            floc = fname.find("shared/")
            code.append("// Declared at " + fname[floc:] + ":" + str(declloc.get("LineNumber")))

        for m in members:
            name = m.get("Name")
            assert name[0:1] == "m"
            otype = m.get("Type").get("Name")
            if otype == "_Bool": otype = "bool"

            plural = "Get" + name[1:]

            # m*Children to GetChild*()
            if name[-8:] == "Children":
                singular = "Get" + name[1:-3]

            # m*Catches to GetCatch*(), m*Classes to GetClass*()
            elif name[-4:] == "ches" or name[-4:] == "shes" or name[-4:] == "sses" or name[-4:] == "xes" :
                singular = "Get" + name[1:-2]

            # mIs* to Is*() for boolean type
            elif name[:3] == "mIs" and otype == "bool":
                plural = name[1:]
                singular = name[1:]

            # Default singular without the endding 's'
            else:
                singular = "Get" + name[1:-1]

            ntype = get_pointed(otype)
            access = m.get("Access")
            accessstr = access if access != None else ""
            if ntype != None:
                if member_functions.get(plural) != None:
                    # gen_call_child_node() for child node in current function body
                    code.append(gen_call_child_node(dictionary, node_name, name, ntype, "node->" + plural + "()"))
                else:
                    # It is an ERROR if no member function for the child node
                    code.append("Error!; // " + gen_call_child_node(dictionary, node_name, name, ntype, "node->" + plural + "()"))
            elif ((otype == "SmallVector" or otype == "SmallList" or otype == "ExprListNode")
                    and member_functions.get(plural + "Num") != None
                    and (member_functions.get(singular) != None or member_functions.get(singular + "AtIndex") != None)):
                func_name = singular if member_functions.get(singular) != None else singular + "AtIndex"
                rtype = member_functions[func_name][2:]
                if rtype == "_Bool": rtype = "bool"
                ntype = get_pointed(rtype)
                if ntype != None or gen_call_handle_values():
                    # gen_call_children_node() for list or vector of nodes before entering the loop
                    code.append(gen_call_children_node(dictionary, node_name, name, otype + "<" + rtype + ">", "node->" + plural + "Num()"))
                    code.append("for(unsigned i = 0; i < node->" + plural + "Num(); ++i) {")
                    if ntype != None:
                        # gen_call_nth_child_node() for the nth child node in the loop for the list or vector
                        code.append(gen_call_nth_child_node(dictionary, node_name, name, ntype, "node->" + func_name + "(i)"))
                    else:
                        # gen_call_nth_child_value() for the nth child value in the loop for the list or vector
                        code.append(gen_call_nth_child_value(dictionary, node_name, name, rtype, "node->" + func_name + "(i)"))
                    code.append("}")
                code.append(gen_call_children_node_end(dictionary, node_name, name, otype + "<" + rtype + ">", "node->" + plural + "Num()"))
            elif gen_call_handle_values():
                if member_functions.get(plural) != None:
                    # gen_call_child_value() for child value in current function body
                    code.append(gen_call_child_value(dictionary, node_name, name, otype, "node->" + plural + "()"))
                else:
                    # It is an ERROR if no member function for the child value
                    code.append("Error!; // " + gen_call_child_value(dictionary, node_name, name, otype, "node->" + plural + "()"))

    # gen_func_definition_end() for the code at the end of current function body
    code.append(gen_func_definition_end(dictionary, node_name))
    append(src_file, code)

    code = []
    code.append(gen_func_declaration(dictionary, node_name))
    append(include_file, code)

# Generate handler for TreeNode
def gen_handler_ast_TreeNode(dictionary):
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
            code.append(gen_call_child_node(dictionary, node_name, "", node_name, "static_cast<" + node_name + "*>(node)"))
        elif node_name == "NullNode":
            code.append("// Ignore NullNode")
        else:
            # it is an ERROR if the node kind is out of range
            code.append("Error!!! // " + gen_call_child_node(dictionary, node_name, "", node_name, "static_cast<" + node_name + "*>(node)"))
        code.append("break;");
    code.append('default: MASSERT(0 && "Unexpected node kind");')
    code.append("}")
    code.append(gen_func_definition_end(dictionary, "TreeNode"))
    append(src_file, code)

# Handle each node which has TreeNode as its base
def gen_handler_ast_node_file(dictionary):
    base = dictionary.get("Bases")
    if base != None:
        basename = base[0].get("Name")
        if basename == "TreeNode":
            gen_handler_ast_node(dictionary)

# Check each child records
def gen_handler(dictionary):
    child_records = dictionary["ChildRecords"]
    for child in child_records:
        value = child["Name"]
        filename = output_dir + 'maplefe/' + value + '.yaml'
        if path.exists(filename):
            handle_yaml(filename, gen_handler_ast_node_file)
    # Generate handler for TreeNode
    gen_handler_ast_TreeNode(dictionary)

###################################################################################################

# Initialize/finalize include_file and src_file with gen_args
Initialization = 1
Finalization = 2
def handle_src_include_files(phase):
    global include_file, src_file, gen_args
    include_file = output_dir + "shared/" + gen_args[0] + ".h"
    src_file = output_dir + "shared/" + gen_args[0] + ".cpp"

    include_start = [
"""
#ifndef __{gen_args1upper}_HEADER__
#define __{gen_args1upper}_HEADER__

#include "ast_module.h"
#include "ast.h"
#include "ast_type.h"
#include "ast_attr.h"
{gen_args3}

namespace maplefe {{

class {gen_args1} {gen_args4} {{
public:
""".format(gen_args1upper=gen_args[1].upper(), gen_args1=gen_args[1], gen_args3=gen_args[3], gen_args4=gen_args[4])
] # include_start

    include_end = [
"""
}};

}}
#endif
""".format() # Use format() to match each pair of "{{" and "}}"
] # include_end

    src_start = [
"""
#include "{gen_args0}.h"

namespace maplefe {{
""".format(gen_args0=gen_args[0])
] # src_start

    src_end = [
"""
}}
""".format() # Use format() to match each pair of "{{" and "}}"
]
    if phase == Initialization:
        create(include_file, license_notice + include_start)
        create(src_file, license_notice + src_start)
    elif phase == Finalization:
        finalize(include_file, include_end)
        finalize(src_file, src_end)

###################################################################################################

def get_data_based_on_type(val_type, accessor):
    if val_type[-10:] == "ASTScope *" or val_type[-12:] == "ASTScopePool":
        return val_type + ': skipped");'
    e = get_enum_type(val_type)
    if e != None:
        return e + ': " + GetEnum' + e + '(' + accessor + '));'
    elif val_type == "LitData":
        return 'LitData: LitId, " + GetEnumLitId(' + accessor + '.mType) + ", " + GetEnumLitData(' + accessor + '));'
    elif val_type == "bool":
        return val_type + ', ", ' + accessor + ');'
    elif val_type == 'unsigned int' or val_type == 'uint32_t' or val_type == 'uint64_t' \
            or val_type == 'unsigned' or val_type == 'int' or val_type == 'int32_t' or val_type == 'int64_t' :
        return val_type + ', " + std::to_string(' + accessor + '));'
    elif val_type == 'const char *':
        return 'const char*, " + (' + accessor + ' ? std::string("\\"") + ' + accessor + ' + "\\"" : "null"));'
    return val_type + ', " + "value"); // Warning: failed to get value'

def short_name(node_type):
    return node_type.replace('class ', '').replace('maplefe::', '').replace(' *', '*')

gen_padding = ''
def padding_name(name):
    return gen_padding + name.ljust(7)

# The follwoing gen_func_* and gen_call* functions are for AstDump
gen_func_decl_location = lambda: True
gen_call_handle_values = lambda: True
gen_func_declaration = lambda dictionary, node_name: \
        "void " + gen_args[2] + node_name + "(" + node_name + "* node);"
gen_func_definition = lambda dictionary, node_name: \
        "void " + gen_args[1] + "::" + gen_args[2] + node_name + "(" + node_name + "* node) {" \
        + ('if (node == nullptr){return;}' if node_name == "TreeNode" else '\nif(DumpFB("' + node_name \
        + '", node)) { MASSERT(node->GetKind() == NK_' + node_name.replace('Node', '') + ');')
gen_call_child_node = lambda dictionary, node_name, field_name, node_type, accessor: \
        ('Dump("' + padding_name(field_name) + ': ' + short_name(node_type) + '*", ' + accessor  + ');\n' \
        if field_name != '' else '') + gen_args[2] + short_name(node_type) + '(' + accessor + ');'
gen_call_child_value = lambda dictionary, node_name, field_name, val_type, accessor: \
        'Dump(std::string("' + padding_name(field_name) + ': ") + "' + get_data_based_on_type(val_type, accessor)
gen_call_children_node = lambda dictionary, node_name, field_name, node_type, accessor: \
        'DumpLB("' + padding_name(field_name) + ': ' + short_name(node_type) + ', size=", ' + accessor+ ');'
gen_call_children_node_end = lambda dictionary, node_name, field_name, node_type, accessor: 'DumpLE(' + accessor + ');'
gen_call_nth_child_node = lambda dictionary, node_name, field_name, node_type, accessor: \
        'Dump(std::to_string(i + 1) + ": ' + short_name(node_type) + '*", ' + accessor + ');\n' \
        + gen_args[2] + short_name(node_type) + '(' + accessor + ');'
gen_call_nth_child_value = lambda dictionary, node_name, field_name, val_type, accessor: \
        'Dump(std::to_string(i) + ". ' + get_data_based_on_type(val_type, accessor)
gen_func_definition_end = lambda dictionary, node_name: \
        'return;\n}' if node_name == "TreeNode" else 'DumpFE();\n}\nreturn;\n}'

#
# Generate source files for dumping AST
#
gen_args = [
        "gen_astdump", # Filename
        "AstDump",     # Class name
        "AstDump",     # Prefix of function name
        "",            # Extra include directives
        "",            # Base class
        ]
astdump = gen_args[0]
astdumpclass = gen_args[1]
prefixfuncname = gen_args[2]

astdump_init = [
"""
private:
ModuleNode    *mASTModule;
std::ostream *mOs;
int           indent;
std::string   indstr;

public:
{gen_args1}(ModuleNode *m) : mASTModule(m), mOs(nullptr), indent(0) {{
indstr = std::string(256, \' \');
for(int i = 2; i < 256; i += 4)
indstr.at(i) = \'.\';
}}

void Dump(const char *title, std::ostream *os) {{
  mOs = os;
  *mOs << "{gen_args1}: " << title << " {{\\n";
  {gen_args2}TreeNode(mASTModule);
  *mOs << "}}\\n";
}}

static std::string GetEnumLitData(LitData lit) {{
  std::string str = GetEnumLitId(lit.mType);
  switch (lit.mType) {{
    case LT_IntegerLiteral:
      return std::to_string(lit.mData.mInt);
    case LT_FPLiteral:
      return std::to_string(lit.mData.mFloat);
    case LT_DoubleLiteral:
      return std::to_string(lit.mData.mDouble);
    case LT_BooleanLiteral:
      return std::string(lit.mData.mBool ? "true" : "false");
    case LT_CharacterLiteral:
      return std::string(1, lit.mData.mChar.mData.mChar); // TODO: Unicode support
    case LT_StringLiteral:
      return std::string(gStringPool.GetStringFromStrIdx(lit.mData.mStrIdx));
    case LT_NullLiteral:
      return std::string("null");
    case LT_ThisLiteral:
      return std::string("this");
    case LT_SuperLiteral:
      return std::string("super");
    case LT_NA:
      return std::string("NA");
    default:;
  }}
}}

private:
void Dump(const std::string& msg) {{
  *mOs << indstr.substr(0, indent) << msg << std::endl;
}}

void Dump(const std::string& msg, TreeNode *node) {{
  *mOs << indstr.substr(0, indent) << msg << (node ? "" : ", null") << std::endl;
}}

void Dump(const std::string& msg, bool val) {{
  *mOs << indstr.substr(0, indent) << msg << (val ? "true" : "false") << std::endl;
}}

TreeNode* DumpFB(const std::string& msg, TreeNode* node) {{
  if (node != nullptr) {{
    *mOs << indstr.substr(0, indent + 2) << msg;
    indent += 4;
    *mOs << " {{" << std::endl;
    DumpTreeNode(node);
  }}
  return node;
}}

void DumpFE() {{
  indent -= 4;
  *mOs << indstr.substr(0, indent + 2) << "}}" << std::endl;
}}

void DumpLB(const std::string& msg, unsigned size) {{
  *mOs << indstr.substr(0, indent) << msg << size << (size ? " [" : "") << std::endl;
  indent += 4;
}}

void DumpLE(unsigned size) {{
  indent -= 4;
  if(size)
    *mOs << indstr.substr(0, indent + 2) << "]" << std::endl;
}}
""".format(gen_args1=gen_args[1], gen_args2=gen_args[2])
] # astdump_init

handle_src_include_files(Initialization)
append(include_file, astdump_init)
handle_yaml(initial_yaml, gen_handler)
append(include_file, ['','public:'])
handle_yaml(initial_yaml, gen_enum_func)
gen_args[2] = "Dump"
gen_padding = "^ "
gen_call_child_node = lambda dictionary, node_name, field_name, node_type, accessor: \
    (('Dump("' + padding_name(field_name) + ': ' + short_name(node_type) \
    + '*, " + (' + accessor + ' ? "NodeId=" + std::to_string(' + accessor \
    + '->GetNodeId()) : std::string("null")));\n' if field_name == "mParent" else \
    'Dump("' + padding_name(field_name) + ': ' + short_name(node_type) + '*", ' + accessor + ');\n' \
    + prefixfuncname + short_name(node_type) + '(' + accessor + ');') if field_name != '' else '')
handle_yaml(treenode_yaml, gen_handler_ast_node)
handle_src_include_files(Finalization)

################################################################################

def gen_setter(accessor):
    return accessor.replace("Get", "Set").replace("()", "(n)").replace("(i)", "(i,n)")

# The follwoing gen_func_* and gen_call* functions are for AstVisitor
gen_call_handle_values = lambda: False
gen_func_declaration = lambda dictionary, node_name: \
        'virtual ' + node_name + '* ' + gen_args[2] + node_name + '(' + node_name + '* node);'
gen_func_definition = lambda dictionary, node_name: \
        node_name + '* ' + gen_args[1] + '::' + gen_args[2] + node_name + '(' + node_name \
        + '* node) {\nif(node != nullptr' + (' && !IsVisited(node)) {' \
        + '\nif(mTrace){std::cout << "Visiting ' + node_name + ', id=" << node->GetNodeId() << "..." << std::endl;}' \
        if node_name != 'TreeNode' else ') {')
gen_call_child_node = lambda dictionary, node_name, field_name, node_type, accessor: \
        'if(auto t = ' + accessor + ') {' + 'auto n = ' + gen_args[2] + node_type + '(t);' \
        + 'if(n != t){' + gen_setter(accessor) + ';}}' if field_name != '' else \
        'return ' + gen_args[2] + node_type + '(' + accessor + ');\n'
gen_call_children_node = lambda dictionary, node_name, field_name, node_type, accessor: ''
gen_call_children_node_end = lambda dictionary, node_name, field_name, node_type, accessor: ''
gen_call_nth_child_node = lambda dictionary, node_name, field_name, node_type, accessor: \
        'if(auto t = ' + accessor + ') { auto n = ' + gen_args[2] + node_type + '(t);' \
        + 'if(n != t) {' + gen_setter(accessor) + ';}}'
gen_func_definition_end = lambda dictionary, node_name: '}\nreturn node;\n}'

# -------------------------------------------------------
gen_args = [
        "gen_astvisitor", # Filename
        "AstVisitor",     # Class name
        "Visit",          # Prefix of function name
        "",               # Extra include directives
        "",               # Base class
        ]
astvisitor = gen_args[0]
astvisitorclass = gen_args[1]

astvisitor_init = [
"""
private:
bool mTrace;
BitVector mVisited;

public:
{gen_args1}(bool t = false) : mTrace(t) {{}}

TreeNode* {gen_args2}(TreeNode* node) {{
  mVisited.ClearAll();
  return {gen_args2}TreeNode(node);
}}

virtual bool IsVisited(TreeNode* node) {{
  if(mVisited.GetBit(node->GetNodeId()))
    return true;
  mVisited.SetBit(node->GetNodeId());
  return false;
}}
""".format(gen_args1=gen_args[1], gen_args2=gen_args[2])
] # astvisitor_init

# Example to extract code pieces starting from initial_yaml
handle_src_include_files(Initialization)
append(include_file, astvisitor_init)
handle_yaml(initial_yaml, gen_handler)
handle_src_include_files(Finalization)

################################################################################

# The follwoing gen_func_* and gen_call* functions are for AstGraph
gen_func_declaration = lambda dictionary, node_name: \
        'void ' + gen_args[2] + node_name + '(' + node_name + '* node);'
gen_func_definition = lambda dictionary, node_name: \
        'void ' + gen_args[1] + '::' + gen_args[2] + node_name + '(' + node_name + '* node) {' \
        + '\nif(node != nullptr' + (' && PutNode(node)) {' \
        + 'if(auto t = node->GetLabel()) { PutEdge(node, t, "Label", NK_Null); DumpGraphTreeNode(t);}' \
        if node_name != "TreeNode" else ') {')
gen_call_child_node = lambda dictionary, node_name, field_name, node_type, accessor: \
        'if(auto t = ' + accessor + ') {' + ('PutEdge(node, t, "' + field_name[1:] + \
        '", NK_' + node_type.replace('Node', '').replace('Tree', 'Null') + ');' \
        if field_name != '' else '') + gen_args[2] + node_type + '(t);}'
gen_call_nth_child_node = lambda dictionary, node_name, field_name, node_type, accessor: \
        'if(auto t = ' + accessor + ') { PutChildEdge(node, t, "' + field_name[1:] \
        + '", i, NK_' + node_type.replace('Node', '').replace('Tree', 'Null') \
        + '); ' + gen_args[2] + node_type + '(t);}'
gen_func_definition_end = lambda dictionary, node_name: '}\n}'

# -------------------------------------------------------
gen_args = [
        "gen_astgraph", # Filename
        "AstGraph",     # Class name
        "DumpGraph",    # Prefix of function name
        """
#include "stringpool.h"
#include "{astdump}.h"
#include <algorithm>
#include <set>
#include <cstring>""".format(astdump = astdump),  # Extra include directives
        "",            # Base class
        ]

astgraph_init = [
"""
{gen_args1}(ModuleNode *m) : mASTModule(m), mOs(nullptr) {{}}

#define NodeName(n,s)  ({astdumpclass}::GetEnumNodeKind((n)->GetKind()) + 3) << s << n->GetNodeId()
#define EnumVal(t,e,m) {astdumpclass}::GetEnum##e((static_cast<t *>(n))->Get##m())
#define NodeColor(c)   "\\",style=filled,color=white,fillcolor=\\""#c

void {gen_args2}(const char *title, std::ostream *os) {{
  mNodes.clear();
  mOs = os;
  auto fn = mASTModule->GetFileName();
  if(auto p = std::strrchr(fn, '/')) fn = p + 1;
  *mOs << "digraph AST_Module {{\\nrankdir=LR;\\nModule [label=\\"Module\\\\n" << fn << "\\\\n" << title << "\\",shape=box];\\n";
  std::size_t idx = 1;
  for(unsigned i = 0; i < mASTModule->GetTreesNum(); i++) {{
    *mOs << "Module -> " << NodeName(mASTModule->GetTree(i),\'_\') << "[label=" << idx++ << "];\\n";
    {gen_args2}TreeNode(mASTModule->GetTree(i));
  }}
  *mOs << "}}\\n";
}}

bool PutNode(TreeNode *n) {{
  if(n && mNodes.find(n) == mNodes.end()) {{
    mNodes.insert(n);
    *mOs << NodeName(n,\'_\') << " [label=\\"" << NodeName(n,',') << "\\\\n";
    switch(n->GetKind()) {{
      case NK_Function:    {{
                             std::string s = n->GetString();
                             *mOs << (n->GetStrIdx() ? s : "_anonymous_") << NodeColor(lightcoral);
                             break;
                           }}
      case NK_Lambda:      *mOs << NodeColor(pink); break;
      case NK_Call:        *mOs << NodeColor(burlywood); break;
      case NK_Block:       *mOs << NodeColor(lightcyan); break;
      case NK_CondBranch:  *mOs << NodeColor(lightblue); break;
      case NK_Return:      *mOs << NodeColor(tan); break;
      case NK_Break:       *mOs << NodeColor(peachpuff); break;
      case NK_Continue:    *mOs << NodeColor(paleturquoise); break;
      case NK_SwitchCase:
      case NK_SwitchLabel:
      case NK_Switch:      *mOs << NodeColor(powderblue); break;
      case NK_ForLoop:     *mOs << EnumVal(ForLoopNode, ForLoopProp, Prop);
      case NK_WhileLoop:
      case NK_DoLoop:      *mOs << NodeColor(lightskyblue); break;
      case NK_Identifier:  *mOs << "\\\\\\"" << n->GetString() << "\\\\\\"" << NodeColor(wheat); break;
      case NK_Decl:        *mOs << EnumVal(DeclNode, DeclProp, Prop) << NodeColor(palegoldenrod); break;
      case NK_PrimType:    *mOs << EnumVal(PrimTypeNode, TypeId, PrimType) << NodeColor(lemonchiffon); break;
      case NK_BinOperator: *mOs << EnumVal(BinOperatorNode, OprId, OprId);
      case NK_TerOperator:
                           *mOs << NodeColor(palegreen); break;
      case NK_UnaOperator: *mOs << EnumVal(UnaOperatorNode, OprId, OprId);
      case NK_InstanceOf:
      case NK_TypeOf:      *mOs << NodeColor(lightgreen); break;
      case NK_Literal:     {{
                             std::string s({astdumpclass}::GetEnumLitData(static_cast<LiteralNode *>(n)->GetData()));
                             std::replace(s.begin(), s.end(), '"', ':');
                             *mOs << s;
                             break;
                           }}
      case NK_Pass:        *mOs << NodeColor(lightgrey); break;
      case NK_New:         *mOs << NodeColor(khaki); break;
      case NK_Try:         *mOs << NodeColor(plum); break;
      case NK_Catch:       *mOs << NodeColor(thistle); break;
      case NK_Finally:     *mOs << NodeColor(thistle); break;
      case NK_Throw:       *mOs << NodeColor(plum); break;
    }}
    if(n->IsStmt())
       *mOs << "\\",penwidth=2,color=\\"tomato";
    *mOs << "\\"];\\n";
    return true;
  }}
  return false;
}}

void PutEdge(TreeNode *from, TreeNode *to, const char *field, NodeKind k) {{
  if(to)
    *mOs << NodeName(from,\'_\') << " -> " << NodeName(to,\'_\') << "[label=" << field << "];\\n";
}}

void PutChildEdge(TreeNode *from, TreeNode *to, const char *field, unsigned idx, NodeKind k) {{
  if(to)
    *mOs << NodeName(from,\'_\') << " -> " << NodeName(to,\'_\') << "[label=\\"" << field << "[" << idx << "]\\""
      << (to->GetKind() == k || k == NK_Null ? "" : ", style=bold, color=red") << "];\\n";
}}

private:
ModuleNode            *mASTModule;
std::ostream         *mOs;
std::set<TreeNode *>  mNodes;
""".format(gen_args1=gen_args[1], gen_args2=gen_args[2], astdumpclass=astdumpclass)
] # astgraph_init

handle_src_include_files(Initialization)
append(include_file, astgraph_init)
handle_yaml(initial_yaml, gen_handler)
handle_src_include_files(Finalization)

###################################################################################################

def get_data_based_on_type(val_type, accessor):
    e = get_enum_type(val_type)
    if e == "ASTScope *":
        return e + ': " + "' + accessor + '");'
    elif e != None:
        return astdumpclass + '::GetEnum' + e + '(' + accessor + ')'
    elif val_type == "LitData":
        return astdumpclass + '::GetEnumLitData(' + accessor + ')'
    elif val_type == "bool":
        return 'std::to_string(' + accessor + ')'
    elif val_type == 'unsigned int' or val_type == 'uint32_t' or val_type == 'uint64_t' \
            or val_type == 'unsigned' or val_type == 'int' or val_type == 'int32_t' or val_type == 'int64_t' :
        return 'std::to_string(' + accessor + ')'
    elif val_type == 'const char *':
        return 'std::to_string(' + accessor + ' ? std::string("\\"") + ' + accessor + ' + "\\"" : "null")'
    return 'Warning: failed to get value with ' + val_type + ", " + accessor

def short_name(node_type):
    return node_type.replace('class ', '').replace('maplefe::', '').replace(' *', '*')

# The follwoing gen_func_* and gen_call* functions are for AstEmitter
gen_func_decl_location = lambda: False
gen_call_handle_values = lambda: True
gen_func_declaration = lambda dictionary, node_name: \
        "std::string " + gen_args[2] + node_name + "(" + node_name + "* node);"
gen_func_definition = lambda dictionary, node_name: \
        "std::string " + gen_args[1] + "::" + gen_args[2] + node_name + "(" + node_name + "* node) {" \
        + 'if (node == nullptr) \nreturn std::string();' \
        + ('' if node_name == "TreeNode" else \
        'std::string str;')
gen_call_child_node = lambda dictionary, node_name, field_name, node_type, accessor: \
        'if(auto n = ' + accessor + ') {str += " "s + ' + gen_args[2] + short_name(node_type) + '(n);}' \
        if field_name != '' else \
        'return ' + gen_args[2] + short_name(node_type) + '(' + accessor + ');'
gen_call_child_value = lambda dictionary, node_name, field_name, val_type, accessor: \
        'str += " "s + ' + get_data_based_on_type(val_type, accessor) + ';'
gen_call_nth_child_node = lambda dictionary, node_name, field_name, node_type, accessor: \
        'if(i)str+= ", "s; if(auto n = ' + accessor + ') {str += " "s + ' + gen_args[2] + short_name(node_type) + '(n);}'
gen_call_nth_child_value = lambda dictionary, node_name, field_name, val_type, accessor: \
        'str += " "s + ' + get_data_based_on_type(val_type, accessor) + ';'
gen_func_definition_end = lambda dictionary, node_name: \
        'mPrecedence = \'\\030\'; if(node->IsStmt()) str += ";\\n"s;' \
        + 'return str;}' if node_name != "TreeNode" else 'return std::string();}'

#
gen_args = [
        "gen_astemitter", # Filename
        "AstEmitter",     # Class name
        "AstEmit",        # Prefix of function name
        """
#include "{astdump}.h"
""".format(astdump = astdump),  # Extra include directives
        ""
        "",            # Base class
        ]

astemit_init = [
"""
using Precedence = char;

private:
ModuleNode    *mASTModule;
std::ostream *mOs;
Precedence    mPrecedence;

public:
{gen_args1}(ModuleNode *m) : mASTModule(m), mOs(nullptr) {{}}

void {gen_args2}(const char *title, std::ostream *os) {{
  mOs = os;
  *mOs << "// [Beginning of {gen_args1}: " << title << "\\n// Filename: " << mASTModule->GetFileName() << "\\n";
  for(unsigned i = 0; i < mASTModule->GetTreesNum(); i++) {{
    *mOs << {gen_args2}TreeNode(mASTModule->GetTree(i));
    *mOs << "// End of AstEmitter]\\n";
  }}

  std::string Clean(std::string &s) {{
    auto len = s.length();
    if(len >= 2 && s.substr(len - 2) == ";\\n")
      return s.erase(len - 2);
    return s;
  }}

""".format(gen_args1=gen_args[1], gen_args2=gen_args[2], astdumpclass=astdumpclass)
] # astemit_init

if False:
    handle_src_include_files(Initialization)
    append(src_file, ['using namespace std::string_literals;'])
    append(include_file, astemit_init)
    handle_yaml(initial_yaml, gen_handler)
    handle_src_include_files(Finalization)

################################################################################

def get_data_based_on_type(val_type, accessor):
    e = get_enum_type(val_type)
    if e == "ASTScope *":
        return '0 /* Error: Should not hit ASTScope. ' + e + ': ' + accessor + ' */'
    elif e != None:
        return 'static_cast<int64_t>(' + accessor + ')'
    elif val_type == "LitData":
        return '0 /* Error: Should not hit LitData */'
    elif val_type == "bool":
        return 'static_cast<int64_t>(' + accessor + ')'
    elif val_type == 'unsigned int' or val_type == 'uint32_t' or val_type == 'uint64_t' \
            or val_type == 'unsigned' or val_type == 'int' or val_type == 'int32_t' or val_type == 'int64_t' :
        return 'static_cast<int64_t>(' + accessor + ')'
    elif val_type == 'const char *':
        return '0 /* Error: Should change const char * to StrIdx */'
    return '0 /* Warning: failed to get value with ' + val_type + ", " + accessor + ' */'

def short_name(node_type):
    return node_type.replace('class ', '').replace('maplefe::', '').replace(' *', '*')

# The follwoing gen_func_* and gen_call* functions are for AstStore
#
gen_func_decl_location = lambda: False
gen_call_handle_values = lambda: True
gen_func_declaration = lambda dictionary, node_name: \
        "void " + gen_args[2] + node_name + "(" + node_name + "* node);"
gen_func_definition = lambda dictionary, node_name: \
        "void " + gen_args[1] + "::" + gen_args[2] + node_name + "(" + node_name + "* node) {" \
        + ('' if node_name == "TreeNode" else 'WriteNum(\'N\', static_cast<int64_t>(node->GetKind()));'
                + 'WriteTreeNode(node); // Base')
gen_call_child_node = lambda dictionary, node_name, field_name, node_type, accessor: \
        'WriteNode(' + accessor + '); // ' + field_name + ': ' + node_type if field_name != '' else \
        gen_args[2] + short_name(node_type) + '(' + accessor + ');'
gen_call_child_value = lambda dictionary, node_name, field_name, val_type, accessor: \
        ('WriteNum(\'V\', static_cast<int64_t>(' + accessor + '.mType));' \
         + 'WriteNum(\'V\', ' + accessor + '.mData.mInt64);' if val_type == 'LitData' else \
         'WriteNum(\'V\', ' + get_data_based_on_type(val_type, accessor) + ');') \
         + ' // ' + field_name + ': ' + val_type
gen_call_children_node = lambda dictionary, node_name, field_name, node_type, accessor: \
        'WriteNum(\'L\', ' + accessor + '); // ' + field_name + ': ' + node_type
gen_call_nth_child_node = lambda dictionary, node_name, field_name, node_type, accessor: \
        'WriteNode(' + accessor + '); // '  + field_name + ': ' + node_type
gen_call_nth_child_value = lambda dictionary, node_name, field_name, val_type, accessor: \
        'WriteNum(\'V\', ' + get_data_based_on_type(val_type, accessor) + '); // ' + field_name + ': ' + val_type
gen_func_definition_end = lambda dictionary, node_name: '}'
#
gen_args = [
        "gen_aststore", # Filename
        "AstStore",     # Class name
        "Store",        # Prefix of function name
        """
#include "{astvisitor}.h"
#include <cstdint>
namespace maplefe {{
using AstBuffer  = std::vector<uint8_t>;
using AstNodeVec = std::vector<TreeNode*>;
}}
""".format(astvisitor=astvisitor),
        ": public " + astvisitorclass,            # Base class
        ]

astemit_init = [
"""
private:
ModuleNode  *mASTModule;
AstBuffer   mAstBuf {{'M', 'P', 'L', 'A', 'S', 'T'}};
BitVector   mVisited;

public:
{gen_args1}(ModuleNode *m) : mASTModule(m) {{}}

const std::vector<uint8_t>& GetAstBuf() const {{return mAstBuf;}}

void {gen_args2}InAstBuf() {{
  mAstBuf.erase(mAstBuf.begin()+6, mAstBuf.end());
  mAstBuf.reserve(65536);
  for(unsigned i = 0; i < mASTModule->GetTreesNum(); i++)
    VisitTreeNode(mASTModule->GetTree(i));
}}

bool IsVisited(TreeNode* node) {{
  if({astvisitorclass}::IsVisited(node))
    return true;
  {gen_args2}TreeNode(node);
  return false;
}}

// Flags:
//   'N': Beginning of a tree node
//   'A': address of a child tree node
//   'V': value of a field in a tree node
//   'L': list/vector of chrildren in a tree node
// The initial version will keep all flags, and some of them can be optimized out

// LEB128, same as for MapleIR
void WriteNum(uint8_t flag, int64_t x) {{
  mAstBuf.push_back(flag);
  while (x < -0x40 || x >= 0x40) {{
    mAstBuf.push_back(static_cast<uint8_t>((static_cast<uint64_t>(x) & 0x7F) + 0x80));
    x = x >> 7;
  }}
  mAstBuf.push_back(static_cast<uint8_t>(static_cast<uint64_t>(x) & 0x7F));
}}

void WriteNode(TreeNode *node) {{
  if(node)
    WriteNum('A', static_cast<int64_t>(node->GetNodeId()));
  else
    WriteNum('A', 0);
}}

""".format(gen_args1=gen_args[1], gen_args2=gen_args[2], astvisitorclass=astvisitorclass)
] # astemit_init

handle_src_include_files(Initialization)
append(src_file, ['using namespace std::string_literals;'])
append(include_file, astemit_init)
handle_yaml(initial_yaml, gen_handler)
gen_args[2] = "Write"
handle_yaml(treenode_yaml, gen_handler_ast_node)
handle_src_include_files(Finalization)
