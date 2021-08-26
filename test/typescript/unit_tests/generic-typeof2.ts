class Klass<T> {
  f: T;
  constructor(t: T) { this.f = t; }
}
class Foo {
  foo: number;
  constructor(n: number) { this.foo = n; }
}
const PROP = "foo";
type FooType = Klass<Foo[typeof PROP]>;
