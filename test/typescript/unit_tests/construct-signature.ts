// https://www.typescriptlang.org/docs/handbook/2/classes.html#abstract-construct-signatures
// used in cocos creator deserialize.ts

// construct signature using generics
// This creates a type alias for <T> constructors that takes no arguments
type T_Ctor<T> = new () => T;

interface I_Class<T> extends T_Ctor<T> {
  __vals__: string[];
}

type AnyCtor = T_Ctor<Object>;
type AnyClass = I_Class<Object>;
