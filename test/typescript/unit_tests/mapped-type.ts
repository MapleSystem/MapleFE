class Base {
  [key: string]: number | string;
}
class Derived extends Base {}
type T<E extends Base> = { [key in E[keyof E]]: string };

var obj: T<Derived> = { str: "abc" };
obj[0] = "zero";
console.log(obj);
