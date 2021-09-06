function class_deco(name: string): Function {
  function deco(ctor: Function): void {
    console.log("Class constructor is :", ctor, ", Name is: ", name);
  }
  return deco;
}
@class_deco("Deco")
class Klass {}
var o = new Klass();
