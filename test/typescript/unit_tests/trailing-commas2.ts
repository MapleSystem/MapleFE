class Klass<T> {
  n: T | undefined = undefined;
}

function func(x = 0.5, y: Klass<number> = { n: 3 }): number {
  return x + y.n!;
}

console.log(func());
