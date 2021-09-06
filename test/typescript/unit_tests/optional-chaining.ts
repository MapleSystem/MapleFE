function func(arr?: number[]) {
  return arr?.[0];
}

let arr: number[] = [1, 2, 3];
console.log(func(arr));
console.log(func());
