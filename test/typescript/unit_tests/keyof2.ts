class Klass {
  [x: string]: number;
}
var o: Klass = { abc: 10 };
var x: keyof typeof o = "abc";
console.log(o[x]);
o[x] = 3;
