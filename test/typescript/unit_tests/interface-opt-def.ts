interface I {
  a: number;
  b?: number;
}

function f({ a, b = 1 }: I): number {
  return a + b;
}

console.log(f({ a: 1 }));
console.log(f({ a: 1, b: 2 }));
