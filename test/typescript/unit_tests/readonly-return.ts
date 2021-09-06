function foo(): readonly number[] {
  var v: number = 1;
  return [v, v];
}

console.log(foo());
