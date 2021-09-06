function nonNullable<T>(e: T): asserts e is NonNullable<T> {
  if (e === null || e === undefined) throw new Error("Assertion failure");
  console.log("nonNullable", e);
}

class Klass {}
var obj: Klass = new Klass();
nonNullable<Klass>(obj);
