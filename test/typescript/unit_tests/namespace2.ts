namespace ns_a {
  var x: number = 10;
  var hello: string = "hello";
  export function foo(): string {
    return hello;
  }
  for (var i = 0; i < x; ++i) console.log(i);
}
console.log(ns_a.foo());
export { ns_a as ns_1 };
