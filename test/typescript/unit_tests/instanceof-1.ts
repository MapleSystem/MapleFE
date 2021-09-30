class Foo {}

class Bar {
  foo: Function = Foo;
}

var foo:Foo = new Foo();
var bar:Bar = new Bar();
console.log(foo instanceof Bar);
console.log(foo instanceof bar.foo);

