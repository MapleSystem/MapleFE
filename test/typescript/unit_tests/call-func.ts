function func(o: Object): Object {
  return o;
}

class Klass {
  [key: string]: number;
  x: number = 123;
}

var obj: Klass = { x: 1 };
(func(obj) as Klass)["x"] = 2;
console.log(obj);
