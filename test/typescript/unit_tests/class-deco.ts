function class_deco(ctor: Function): void {
  console.log("Class constructor is :", ctor);
}
@class_deco
class Klass {}
var o = new Klass();
