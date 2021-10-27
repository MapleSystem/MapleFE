function* delegate(num: number) : Generator<number> {
  let r = yield num + 100;
  return r;
}

function* gen(n: number): Generator<number, string, boolean> {
  for (var i = 1; i <= n; i++) {
    let res = yield* delegate(i);
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
