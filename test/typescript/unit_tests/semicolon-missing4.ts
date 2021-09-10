class Klass {
  func?: (n: number) => number;
}
var obj: Klass = new Klass();
obj.func = function (n: number): number {
  return n;
};

export function show() {
  console.log(obj.func!(123));
}
