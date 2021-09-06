for (var i: number = 1; i < 10; ++i) {
  if (i < 5) {
    console.log(i, " is less than 5");
    continue;
  }
  if (i == 8) break;
  console.log(i, " is greater than 4");
}
