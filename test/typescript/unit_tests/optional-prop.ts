class Klass {
  a: number;
  b?: number;
}

function f({ a, b = 1 }: Klass): number {
  return a + b;
}

console.log(f({ a: 1 }));
console.log(f({ a: 1, b: 2 }));
