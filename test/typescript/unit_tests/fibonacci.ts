function fibonacci(m: number) {
  var f0: number = 0;
  var f1: number = 1;
  var f2: number;
  var i: number;
  if (m <= 1) {
    return m;
  } else {
    for (i = 2; i <= m; i++) {
      f2 = f0 + f1;
      f0 = f1;
      f1 = f2;
    }
    return f2;
  }
}
