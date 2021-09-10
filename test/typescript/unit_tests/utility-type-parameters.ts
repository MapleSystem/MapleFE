var list = { f1: 0, f2: "abc" };
function f(...list: any[]) {}
function func(t: Parameters<typeof f>[0]): Parameters<typeof f>[1] {
  return list.f2 + t;
}
console.log(func(123));
