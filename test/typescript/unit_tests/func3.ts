interface Foo {
  val1: number;
  val2: number;
};

function func(this: Foo , arg: number) {
  var x:number;
  this.val1 = arg;
  x = this.val1;
  this.val2 = x;
}

var a: Foo = {val1:0, val2:0};
var b = new (func as any)(456);  // allocate an object and call func as constructor
func.call(a, 123);               // pass an existing object to func
console.log(a);
console.log(b);
