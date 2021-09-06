for (var i: number = 1; i < 10; ++i) {
  if (i < 5)
    switch (i) {
      case 2:
        console.log(i, ", continue");
        continue;
      case 3:
        console.log(i, ", fall-through");
      case 4:
        console.log(i, ", break");
        break;
      default:
        console.log(i, ", default");
    }
  else {
    console.log(i, ", else branch");
    break;
  }
}
