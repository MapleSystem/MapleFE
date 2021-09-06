function foo(j: number): number {
  {
    const i: number = 4;
    j += 10 * i;
  }
  var i = 8;
  return i + j;
}

console.log(foo(5));
