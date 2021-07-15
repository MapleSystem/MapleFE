class Base { [key: string]: number | string }
class Derived extends Base {}
type T<E extends Base> = { [key in E[keyof E]]: string };

var x: T<Derived> = { val: "abc" };
console.log(x);
