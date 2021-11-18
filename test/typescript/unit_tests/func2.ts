function func(this: any, arg: number) {
  var x:number;
  this.val1 = arg;
  x = this.val1;
  this.val2 = x;
}

var a = {};
var b = new (func as any)(456);  // allocate an object and call func as constructor
func.call(a, 123);               // pass an existing object to func
console.log(a);
console.log(b);
