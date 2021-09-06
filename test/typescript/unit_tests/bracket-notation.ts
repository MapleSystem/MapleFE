interface Foo {
  [key: string]: number;
}

let bar: Foo = {};
bar["key1"] = 1;
console.log(bar["key1"]);
