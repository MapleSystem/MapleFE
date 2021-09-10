class Foo {
  f1: number = 0;
  f2: string = "";
}

class Bar {
  f1: number = 0;
  f2: string = "";
}

var x = { f1: 123, f2: "John" };
var y: Foo;
var z: Bar;
y = x;
z = y;
console.log(y, z);
