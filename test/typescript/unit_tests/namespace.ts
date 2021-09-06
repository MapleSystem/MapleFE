namespace ns_a {
  var hello: string = "hello";
  export function foo(): string {
    return hello;
  }
}
console.log(ns_a.foo());
export { ns_a as ns_1 };
