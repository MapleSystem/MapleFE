class Bar {
  public b1: number = 5;
  public b2: number = 6;
}

class Foo {
  public f1: number = 0;
  public f2: number = 0;
  public bar: Bar;
  constructor(a: number, b: number) {
    this.f1 = a;
    this.f2 = b;
    this.bar= new Bar();
  }
}

class App {
  public foo: Foo;
  constructor(a: number, b: number) {
    this.foo = new Foo(a, b);
  }
}

var foo: Foo = new Foo(1, 2);
console.log(foo);

var app: App = new App(3, 4);
console.log(app);


