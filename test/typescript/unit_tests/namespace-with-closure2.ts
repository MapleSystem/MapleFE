namespace ns {
  let i: number = 10;
  export function func() : Function {
    return () => ++i;
  }
}

import myFunc = ns.func;

var f: Function = myFunc();
console.log(f());
console.log(f());

f = ns.func();
console.log(f());
console.log(f());
