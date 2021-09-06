type UnionType = number | string;
function add(x: UnionType, y: UnionType): UnionType {
  if (typeof x === "number" && typeof y == "number") return x + y;
  else return x.toString() + y.toString();
}
console.log(add(1, 2));
console.log(add("a", "b"));
