var x: number = 10;
for (var i = 0; i < 10; ++i) {
  x--;
  outer: if (x > 0) {
    if (x > 5) {
      if (x > 8) {
        console.log("x is greater than 8");
        break outer;
      }
      console.log("x is greater than 5");
      break;
    } else console.log("x is less than or equal to 5");
    console.log("out of nested if-stmt");
  }
}
