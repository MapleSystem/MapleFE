class Klass {
  func(f: () => void): void {
    f();
  }
  public abstract func2(f: () => void): void;
}

function foo() : void {
  console.log("foo");
}

var obj: Klass = new Klass();
obj.func(foo);

