class Klass<T> {
  f: T | undefined = undefined;
}

const t = new Klass<number>();
t.f = 123;
console.log(typeof t);

export type MyType = Klass<typeof t>;
