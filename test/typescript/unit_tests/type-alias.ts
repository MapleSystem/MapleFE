type Foo = { name: "foo" };
type Bar = { name: "bar" };
type Tee = Foo | Bar;

var f: Tee = { name: "foo" };
var b: Tee = { name: "bar" };
console.log(f, b);
