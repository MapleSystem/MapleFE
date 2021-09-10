class MyClass {
  operand: number;

  constructor(opr: number) {
    this.operand = opr;
  }

  calc?: (x: number) => number;

  add = (x: number): number => this.operand + x;
}

MyClass.prototype.calc = function (this: MyClass, x: number) {
  return this.operand + x;
};

var myObj = new MyClass(1);

console.log(myObj.calc!(2));
console.log(myObj.add(3));
