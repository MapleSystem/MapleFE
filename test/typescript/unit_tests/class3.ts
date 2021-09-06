class Foo {
  public out: number = 0;
  public in: number = 0;
  constructor(a: number, b: number) {
    this.out = a;
    this.in = b;
  }
}

var obj: Foo = new Foo(12, 34);
console.log(obj.in);
