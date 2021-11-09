function func(arr: Array<number>): number {
  var len = arr.length;
  var sum = 0;
  while(len > 0) {
    len--
    sum += arr[len];
  }
  return sum;
}

console.log(func([1, 2, 3, 4]));
