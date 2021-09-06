function func(x: unknown): number | undefined {
  if (typeof x === "number") return x * x;
  return undefined;
}

let v = func("a");
console.log(v);
v = func(3);
console.log(v);
