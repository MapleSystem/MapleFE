function prop_deco(msg: string) {
  return function (target: any, name: string) {
    console.log("Accessed", name, msg, target);
  };
}

class Klass {
  _x: number;
  constructor(i: number) {
    this._x = i;
  }

  @prop_deco("of")
  get x() {
    return this._x;
  }
}

var c = new Klass(3);
console.log(c.x);
