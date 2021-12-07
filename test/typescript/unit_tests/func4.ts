class Foo {
  public val1: number = 0;
  public val2: number = 0;
};

function func(this: Foo , arg: number) {
  var x:number;
  this.val1 = arg;
  x = this.val1;
  this.val2 = x;
}

var a = new  Foo();
var b = new (func as any)(456);  // allocate an object and call func as constructor
func.call(a, 123);               // pass an existing object to func
console.log(a);
console.log(b);
