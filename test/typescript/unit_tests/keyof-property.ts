class Klass {
  prop: { field: number; };
}
export type T = keyof Klass["prop"];
var n: T = "field";
console.log(n);

