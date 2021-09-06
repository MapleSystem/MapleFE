var arr: number[] = [7, 4, 5, 9, 2, 8, 1, 6, 3];
var len: number = arr.length;
var sum: number = 0;
outer: for (var i = 0; i < len; ++i) {
  for (var j = i + 1; j < len; ++j) {
    sum += arr[i];
    console.log(sum);
    if (arr[j] > 5) continue outer;
    if (sum >= 60) break outer;
  }
}
console.log(sum);
