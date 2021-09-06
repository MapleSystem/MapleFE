function func(o: Object): Object {
  return o;
}

var obj = { x: 1 };
func(obj)["x"] = 2;
console.log(obj);
