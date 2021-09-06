/* 1. class factory */

// Generic constructor interface
type Constructor<T = {}> = new (...args: any[]) => T;

// A standard interface to be incorporated into all generated classes
interface someStandardInterface {}

// Class factory that takes a base class and generates a new class with a standard interface
function ClassFactory<TBase>(
  base: Constructor<TBase>
): Constructor<TBase & someStandardInterface> {
  class GeneratedClass extends (base as unknown as any) {}

  return GeneratedClass as unknown as any;
}

/* 2. Usage of the class factory */

// A class to be use as base for generating one with some standard interface
class someBaseClass {}

// A class that uses ClassFactory to generate a new class with standard inteface
class newClassWithStandardInterface extends ClassFactory(someBaseClass) {}
