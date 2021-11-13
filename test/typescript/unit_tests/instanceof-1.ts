class Foo {}

class Bar {
  foo: Function = () => null;
}

var foo:Foo = new Foo();
var bar:Bar = new Bar();
bar.foo = Foo;
console.log(foo instanceof Bar);
console.log(foo instanceof bar.foo);
console.log(bar.foo instanceof Bar);

