var arr: number[] = [7, 4, 5, 9, 2, 8, 1, 6, 3];
var len: number = arr.length;
for (var i = 1; i < len; ++i) {
  var j: number = i - 1;
  var n = arr[i];
  while (n < arr[j] && j >= 0) {
    arr[j + 1] = arr[j];
    --j;
  }
  arr[j + 1] = n;
}
console.log(arr);
