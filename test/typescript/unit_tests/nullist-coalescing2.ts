function func(x: any): any {
  return x;
}
let a: any = { f: false };
// https://www.typescriptlang.org/docs/handbook/release-notes/typescript-3-7.html#optional-chaining
let b: boolean = func(a)?.f ?? true;
console.log(a, b);
