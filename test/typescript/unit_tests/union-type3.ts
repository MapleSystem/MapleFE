function func(k: number): number {
  return k + k;
}

type UT = string | typeof func;
var s: UT = "abc";
console.log(s);
s = func;
console.log(s, s(3));
