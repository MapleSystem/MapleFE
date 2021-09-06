var x: number = 10;
if (x > 0) {
  outer: if (x > 5) {
    if (x > 8) {
      console.log("x is greater than 8");
      break outer;
    }
    console.log("x is greater than 5");
  } else console.log("x is less than or equal to 5");
}
