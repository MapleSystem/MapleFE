class Base {
  str: string;
  constructor(s: string) { this.str = s; }
}

class Derived extends Base {
  num: number;
  constructor(s: string, n: number) { super(s); this.num = n; } 
}

function func(...args: Base[]): Derived {
  if (args.length === 1 && args[0] instanceof Derived) {
    return <Derived>args[0];
  }
  return { str: "Unkown", num: 0 }; 
}

var b: Base = new Base("Base");
console.log(func(b));
var d: Derived = new Derived("Derived", 123);
console.log(func(d));
