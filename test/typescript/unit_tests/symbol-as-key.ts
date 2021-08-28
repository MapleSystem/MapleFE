// tsc --target es2015
const tag: unique symbol = Symbol("my symbol");
interface IFace {
    [tag]?: number;
}

class Klass implements IFace {
  [tag]: number;
  s: string = "example";
}

var obj: Klass = new Klass();
obj[tag] = 123;
console.log(obj);
console.log(Symbol.keyFor(tag));

const shared: symbol = Symbol.for("shared symbol");
console.log(Symbol.keyFor(shared));
