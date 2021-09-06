function func(x: number): number {
  return (x > 10 ? x : x * x) ? x * 2 : x * 3;
}

let r = func(12);
console.log(r);
r = func(3);
console.log(r);
