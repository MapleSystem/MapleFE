class Klass {
  prop: { field: number } = { field: 0};
}
export type T = keyof Klass["prop"];
var n: T = "field";
console.log(n);
