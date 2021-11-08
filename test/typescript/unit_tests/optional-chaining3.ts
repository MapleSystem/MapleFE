const obj = {
  func: function () {
    return 123;
  },
};

function foo() {
  return obj;
}
console.log(foo()?.func?.());
