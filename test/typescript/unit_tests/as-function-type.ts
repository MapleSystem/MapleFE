function func(cb: (s: string) => number): string {
  console.log(cb("abc"));
  return "OK";
}

var fn = function (s: string): number {
  console.log(s);
  return 123;
};

const f = func(fn as (s: string) => number);
console.log(f);
