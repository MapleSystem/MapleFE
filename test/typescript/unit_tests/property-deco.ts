function prop_deco(msg: string) {
  return function (target: any, name: string) {
    console.log("Accessed", name, msg, target);
  };
}

class Klass {
  @prop_deco("of")
  x: number;
  constructor(i: number) {
    this.x = i;
  }
}

var c = new Klass(3);
console.log(c.x);
