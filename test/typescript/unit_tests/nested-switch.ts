var n: number = 3;
outer: switch (true) {
  case n < 5:
    console.log(n, " is less than 5");
  case n > 2 && n < 5:
    switch (n) {
      case 3:
        console.log(3);
        break outer;
      case 4:
        console.log(4);
    }
    console.log(n, " + 1 is equal to", n + 1);
    break;
  case n == 6:
    console.log(n, " is equal to 6");
    break;
  case n < 8:
    console.log(n, " is greater than 4 and less than 8");
    break;
  default:
    console.log(n, " is greater than 7");
}
