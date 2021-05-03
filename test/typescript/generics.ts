function foo<T>(arg: T): T {
  return arg;
}

let output = foo<string>("abc");

console.log(output);
