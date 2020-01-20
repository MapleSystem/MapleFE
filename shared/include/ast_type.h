///////////////////////////////////////////////////////////////////////////////
// We need give each symbol, expression a type. The type in AST doesn't have
// any physical representation. It's merely for syntax validation and potential
// optimizations for future R&D project.
//
// Each language should define its own type system, and its own validation rules.
// We only define two categories of types in this file, Primitive and Named types.
///////////////////////////////////////////////////////////////////////////////

#ifndef __AST_TYPE_H__
#define __AST_TYPE_H__

enum TypeCategory {
  Primitive,
  Named
};

class TreeType {
public:
  TypeCategory mCat;
public:
  const char* GetName();  // type name
};

#endif
