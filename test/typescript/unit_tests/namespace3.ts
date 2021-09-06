namespace ns.a {
  var hello: string = "hello";
  export function foo(): string {
    return hello;
  }
}
console.log(ns.a.foo());
