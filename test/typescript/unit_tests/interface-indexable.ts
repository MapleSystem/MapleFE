const enum TypeID {
  Array_Class = 0,
  Array,
}

class Klass {}

interface DataTypes {
  [TypeID.Array_Class]: DataTypes[TypeID.Array][];
  [TypeID.Array]: Klass;
}

type TYPE = DataTypes[Exclude<keyof DataTypes, TypeID.Array_Class>];
