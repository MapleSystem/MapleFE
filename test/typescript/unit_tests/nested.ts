let x: number = 10;

function outer(y: number): number {
  let x: number = 20;
  function inner(): number {
    return x + y;
  }
  return inner();
}

console.log(outer(100));
