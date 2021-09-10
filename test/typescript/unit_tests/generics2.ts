class E<T> {
  t: T | undefined = undefined;
}

class Klass {
  public n: E<{ s: string }> | undefined = undefined;
}

var obj: Klass = new Klass();
obj.n = { t: { s: "example" } };
console.log(obj);
