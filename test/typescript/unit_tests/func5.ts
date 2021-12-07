function func(arg: number): number {
  return arg;;
}

var a = new (func as any)(123);
console.log(a);
console.log(func(123));

