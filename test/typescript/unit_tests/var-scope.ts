var i: number = 5;

function foo(j: number): number {
  i = j;
  {
    let i: number = 2;
    j += 100 * i;
  }
  {
    let i: number = 4;
    j += 10 * i;
  }
  return i + j;
}

console.log(foo(5));
