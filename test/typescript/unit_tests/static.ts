class Foo {
  static x: number = 1;
  static inc(y: number) {
    return this.x + y;
  }
}

console.log(Foo.x);
console.log(Foo.inc(10));
