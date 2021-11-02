function func(args: any): any {
  const {foo, ...others} = args;
  return others;
}

console.log(func({foo: "Foo", bar: "Bar", tee: "Tee"}));
