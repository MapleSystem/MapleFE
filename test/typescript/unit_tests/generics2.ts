class E<T> {
  t : T;
}

class Klass {
    public n: E<{ s: string }>;
}

var obj : Klass = new Klass();
obj.n = { t: { s: "example" }, };
console.log(obj);
