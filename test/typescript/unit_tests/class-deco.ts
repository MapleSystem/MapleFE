function class_deco(ctor): void {
  console.log("Class constructor is :", ctor);
}
@class_deco
class Klass {}
var o = new Klass();
