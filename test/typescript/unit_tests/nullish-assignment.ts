function func(num?: number): number {
  num ??= 2;
  return num * num;
}
console.log(func(10));
console.log(func());
