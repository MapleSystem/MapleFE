// https://www.typescriptlang.org/docs/handbook/release-notes/typescript-3-7.html#nullish-coalescing
let a: any = null;
let b: boolean = a ?? true;
console.log(a, b);
