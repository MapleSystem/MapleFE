class Foo {
  public s: string;
  constructor(f: (args?: [d: string, i: string], obj?: string) => string) {
    this.s = f();
  }
}

function func(args: [d: string, i: string], obj?: string): string {
  return "abc";
}
var obj: Foo = new Foo(func);
console.log(obj.s);
