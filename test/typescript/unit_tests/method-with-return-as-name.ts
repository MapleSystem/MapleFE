class Klass {
  return(x: number) {
    console.log(x);
  }
  foo(x: number) {
    this.return(x);
  }
}

var obj: Klass = new Klass();
obj.foo(10);
