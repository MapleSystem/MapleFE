function* gen(n: number): Generator<number, string, boolean> {
  for (var i = 1; i <= n; i++) {
    let res = yield i;
    console.log(res);
    if (res)
      break;
  }
  return "done";
}

const obj: Generator<number, string, boolean> = gen(10);
console.log(obj.next());
console.log(obj.next(false));
console.log(obj.next(true));
console.log(obj.next());
