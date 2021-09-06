interface Base {
  name: string;
}
interface Derived extends Base {
  age: number;
  readonly impl?: any;
  initialize?(c: number): void;
}
function dump(obj: Derived) {
  console.log(obj.name, obj.age);
}

let o = { name: "John", age: 30 };
dump(o);
