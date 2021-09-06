abstract class Base {
  func(f: () => void): void {
    f();
  }
  public abstract func2(f: () => void): void;
}

class Klass extends Base {
  func2(f: () => void): void {
    f();
  }
}

function foo(): void {
  console.log("foo");
}

var obj: Klass = new Klass();
obj.func(foo);
