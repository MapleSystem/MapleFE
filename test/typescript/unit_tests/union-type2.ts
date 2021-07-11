type UT = string | ((k: number) => number);
var s: UT = "abc";
console.log(s);
s = function func(k: number): number {
  return k + k;
};
console.log(s, s(3));
