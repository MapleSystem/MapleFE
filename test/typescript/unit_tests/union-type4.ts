function func(k: number): number {
  return k + k;
}

type UT = string | typeof func | Array<string>;
var s: UT = "abc";
console.log(s);
s = func;
console.log(s, s(3));
s = new Array("first", "second");
console.log(s);
