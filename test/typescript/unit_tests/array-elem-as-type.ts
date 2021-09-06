var arr: number[] = [7, 4, 5, 9, 2, 8, 1, 6, 3];
var sum: number = 0;
var i;
var len;
(i = 0), (len = arr.length);
for (; i < len; ++i) {
  var x = arr[i] as number;
  sum += x;
}
console.log(sum);
