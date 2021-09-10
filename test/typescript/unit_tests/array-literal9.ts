class Foo {
  [key: string]: number;
  public f1: number = 0;
  private f2: number = 0;
  constructor(a: number, b: number) {
    this.f1 = a;
    this.f2 = b;
  }
}
function func(): Foo {
  console.log("Returning a new object");
  return new Foo(789, 0);
}
var obj: Foo | undefined = undefined;
var i: number = 1;
var res: number = (obj || func())[`f${i}`];
console.log(res);
obj = new Foo(123, 456);
++i;
res = (obj || func())[`f${i}`];
console.log(res);
