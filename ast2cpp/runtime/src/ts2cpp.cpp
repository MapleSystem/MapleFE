#include <algorithm>
#include <sstream>
#include "../include/ts2cpp.h"

std::ostream& operator<< (std::ostream& out, const t2crt::JS_Val& v) {
  switch(v.type) {
    case t2crt::TY_None: out << "None"; break;
    case t2crt::TY_Undef: out << "undefined"; break;
    case t2crt::TY_Null: out << "null"; break;
    case t2crt::TY_Bool: out << v.x.val_bool; break;
    case t2crt::TY_Long: out << v.x.val_long; break;
    case t2crt::TY_Double: out << v.x.val_double; break;
    case t2crt::TY_BigInt: out << "bigint"; break;
    case t2crt::TY_String: out << *v.x.val_string; break;
    case t2crt::TY_Symbol: out << "symbol"; break;
    case t2crt::TY_Function: out << "function"; break;
    case t2crt::TY_Object: out << v.x.val_obj; break;

    case t2crt::TY_CXX_Undef: out  << "undefined"; break;
    case t2crt::TY_CXX_Null: out   << "null"; break;
    case t2crt::TY_CXX_Bool: out   << *(bool*)v.x.field; break;
    case t2crt::TY_CXX_Long: out   << *(int64_t*)v.x.field; break;
    case t2crt::TY_CXX_Double: out << *(double *)v.x.field; break;
    case t2crt::TY_CXX_BigInt: out << "bigint"; break;
    case t2crt::TY_CXX_String: out << *(*(std::string**)v.x.field); break;
    case t2crt::TY_CXX_Symbol: out << "symbol"; break;
    case t2crt::TY_CXX_Function: out << "function"; break;
    case t2crt::TY_CXX_Object: out << *(Object**)v.x.field; break;
 }
  return out;
}

std::ostream& operator<< (std::ostream& out, const t2crt::Object *obj) {
  out << obj->__GetClassName();
  if (obj->IsEmpty())
    out << "{}";
  else {
    std::vector<std::string> vec;
    unsigned cnt = 0;
    // non-object fields go first
    for (bool flag: { false, true }) {
      for (auto it = obj->propList.begin(); it != obj->propList.end(); it++) {
        std::stringstream buf;
        auto k = it->second.type;
        auto b = k == t2crt::TY_Object || k == t2crt::TY_CXX_Object;
        if (b == flag) {
          if (isdigit(it->first.front()))
            buf << '\'' << it->first << "': ";
          else
            buf << it->first << ": ";
          if (k == t2crt::TY_String || k == t2crt::TY_CXX_String)
            buf << '\'' << it->second << '\'';
          else
            buf << it->second;
          vec.push_back(buf.str());
        }
      }
      std::sort(vec.begin() + cnt, vec.end());
      cnt = vec.size();
    }
    const char *p = "{ ";
    for (auto prop: vec) {
      out << p << prop;
      p = ", ";
    }
    out << " }";
  }
  return out;
}

const t2crt::JS_Val undefined = { 0, t2crt::TY_Undef, false };
const t2crt::JS_Val null = { 0, t2crt::TY_Null, false };

namespace t2crt {

bool InstanceOf(JS_Val val, Function* ctor) {
  if (val.type != TY_Object || val.x.val_obj == nullptr || ctor == nullptr)
    return false;

  Object* p = val.x.val_obj->__proto__;
  while (p) {
    if (p == ctor->prototype)
      return true;
    else
      p = p->__proto__;
  }
  return false;
}

// Generate DOT graph output to show object inheritance with
// constructor, prototype chain and prototype property linkages
void GenerateDOTGraph( std::vector<Object *>&obj, std::vector<std::string>&name) {
  for(int g = 0; g < 2; ++g) {
    std::vector<Object*> objs = obj;
    std::vector<std::string> names = name;
    std::cout << "digraph JS" << g << " {\nranksep=0.6;\nnodesep=0.6;\n" << (g == 0 ? "newrank=true;\n" : "") << std::endl;
    int num = objs.size();
    int k = num;

    for (int i=0; i<num; ++i) {
      bool isFuncObj = objs[i]->IsFuncObj();
      if (isFuncObj) {
        // only function objects have prototype prop
        objs.push_back(((Function *)objs[i])->prototype);
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
        if(g > 0 && objs[i]->constructor == objs[j])
          std::cout << names[i] << " -> " << names[j] << " [label=\"ctor\", color=darkgreen, fontcolor=darkgreen];" << std::endl;

        if(objs[i]->__proto__ == objs[j])
          std::cout << names[i] << " -> " << names[j] << " [label=\"__proto__\", color=red, fontcolor=red];" << std::endl;
      }
    }
    std::cout << "}" << std::endl;
  }
}

} // namespace t2crt
