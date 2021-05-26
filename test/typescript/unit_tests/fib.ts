var m = 5;
var f0 = 0;
var f1 = 1;
var f2;
var i;
if (m <= 1) {
  return m;
}
else {
  for (i = 2; i <= m; i++) {
    f2 = f0 + f1;
    f0 = f1;
    f1 = f2;
  }
  return f2;
}
