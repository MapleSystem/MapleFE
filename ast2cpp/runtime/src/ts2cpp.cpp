#include "../include/ts2cpp.h"

namespace t2crt {

const t2crt::JS_Val undefined = { 0, t2crt::TY_Undef, false };

std::ostream& operator<< (std::ostream& out, const t2crt::JS_Val& v) {
  switch(v.type) {
    case t2crt::TY_None: out << "None"; break;
    case t2crt::TY_Undef: out << "undefined"; break;
    case t2crt::TY_Null: out << "null"; break;
    case t2crt::TY_Bool: out << v.x.val_bool; break;
    case t2crt::TY_Long: out << v.x.val_long; break;
    case t2crt::TY_Double: out << v.x.val_double; break;
    case t2crt::TY_BigInt: out << "bigint"; break;
    case t2crt::TY_String: out << "string"; break;
    case t2crt::TY_Symbol: out << "symbol"; break;
    case t2crt::TY_Function: out << "function"; break;
    case t2crt::TY_Object: out << "object"; break;
  }
  return out;
}

// Generate DOT graph output to show object inheritance with 
// constructor, prototype chain and prototype property linkages
void GenerateDOTGraph( std::vector<BaseObj *>&obj, std::vector<std::string>&name) {
  for(int g = 0; g < 2; ++g) {
    std::vector<BaseObj*> objs = obj;
    std::vector<std::string> names = name;
    std::cout << "digraph JS" << g << " {\nranksep=0.6;\nnodesep=0.6;\n" << (g == 0 ? "newrank=true;\n" : "") << std::endl;
    int num = objs.size();
    int k = num;

    for (int i=0; i<num; ++i) {
      bool isFuncObj = objs[i]->IsFuncObj();
      if (isFuncObj) {
        // only function objects have prototype prop
        objs.push_back(((Ctor *)objs[i])->_prototype);
        names.push_back(names[i] + "_prototype");
        if (g == 0) {
          std::cout << "subgraph cluster_" << names[i] << " {\nrank=same;\ncolor=white;\n"
          << names[i] << ";\n" << names[k] << "[label=\"" << names[i] << ".prototype\", shape=box];\n }" << std::endl;
        }
        else {
          std::cout <<  names[k] << "[label=\"" << names[i] << ".prototype\", shape=box];" << std::endl;
        }
        std::cout << names[i] << " -> " << names[k] << " [label=\"prototype\", color=blue, fontcolor=blue];" << std::endl;
        k++;
      }
    }
    num = objs.size();
    for (int i=0; i<num; ++i) {
      for (int j=0; j<num; ++j) {
        if(g > 0 && objs[i]->_ctor== objs[j])
          std::cout << names[i] << " -> " << names[j] << " [label=\"ctor\", color=darkgreen, fontcolor=darkgreen];" << std::endl;

        if(objs[i]->_proto == objs[j])
          std::cout << names[i] << " -> " << names[j] << " [label=\"__proto__\", color=red, fontcolor=red];" << std::endl;
      }
    }
    std::cout << "}" << std::endl;
  }
}

} // namespace t2crt
