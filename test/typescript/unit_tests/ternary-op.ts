function func(x: number): number {
  return x > 10 ? x : x * x;
}

let r = func(12);
console.log(r);
r = func(3);
console.log(r);
