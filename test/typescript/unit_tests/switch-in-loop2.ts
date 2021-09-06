function func() {
  var obj = {};
  for (var i: number = 1; i < 4; ++i) {
    switch (i) {
      case 2:
        break;
      default:
        obj[i] = "OK";
        break;
    }
  }
  return obj;
}

console.log(func());
