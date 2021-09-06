function func(t: Function | [Function] | any): string {
  return typeof t;
}

var f: Function = func;
console.log(func(f));
