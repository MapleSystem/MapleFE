interface Base {
  name: string;
}
interface Derived extends Base {
  age: number;
}

let o = { name: "John", age: 30 };

function dump(obj: Base) {
  console.log(obj.name, (<Derived>obj).age);
}
dump(o);

function dump2(obj: Base) {
  console.log(obj.name, (obj as Derived).age);
}
dump2(o);
