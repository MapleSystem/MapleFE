export { a, o, func, u as U, v as V };

var a: number[] = [1, 2, 3];
console.log(a.length);

var o: Object = { x: 1, y: "abc" };
console.log(o);

function func(n: number): number {
  return n * n;
}
console.log(func(10));

var [u, v] = a;
console.log(u, v);
