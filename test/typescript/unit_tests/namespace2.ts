namespace ns_a {
 var x: number = 10;
 var hello : string = "hello";
 export function foo() : string {
   return hello;
 }
}
console.log(ns_a.foo());
namespace ns_a {
  for(var i = 0; i < 10; ++i)
    console.log(i);
}
export {ns_a as ns_1};
