function type(msg: string) {
  return function (target: any, name: string) {
    console.log("Accessed", name, msg, target);
  };
}

function deco(target: any, name: string) {
  console.log("Accessed", name, "OF", target);
}

class Klass {
  @deco
  @type("of")
  x: number;
  constructor(i: number) {
    this.x = i;
  }
}

var c = new Klass(3);
console.log(c.x);
