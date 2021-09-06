class Foo {
  public a: string;
}

function func(x?: Foo): Foo {
  return x ? x : { a: "default:a" };
}

console.log(func());
console.log(func({ a: "Foo:a" }));
