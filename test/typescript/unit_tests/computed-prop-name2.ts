function func() {
  return {[prop] : "abc" };
}

var prop: string | number = "my key";
console.log(func());
prop = -12.3;
console.log(func());

