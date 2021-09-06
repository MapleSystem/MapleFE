class Klass {}
class Foo {
  public f1: number = 0;
  private f2: number = 0;
  constructor(a: number, b: number) {
    this.f1 = a;
    this.f2 = b;
  }
  public static test(obj: unknown): obj is Klass {
    return obj instanceof Klass;
  }
}

var obj: Klass = new Klass();
console.log(Foo.test(obj));
