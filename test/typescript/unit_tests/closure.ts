var base: number = 10;

function counter(private_count: number): () => number {
  function increment(): number {
    private_count += 1;
    return base + private_count;
  }

  return increment;
}

var count: () => number = counter(0);
console.log(count()); // output: 11
console.log(count()); // output: 12
var count2: () => number = counter(100);
console.log(count2()); // output: 111
console.log(count2()); // output: 112
