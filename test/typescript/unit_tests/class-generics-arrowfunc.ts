// Class with generics and arrow function.
// - Class constructor takes a function as parameter and saves it.
// - The saved function allocates and return an object with generics type.
// - Class method alloc() calls the saved function to allocate and return an obj.
class Foo<T> {
  private _ctor: () => T;

  constructor(ctor: () => T) {
    this._ctor = ctor;
  }

  public alloc(): T {
    return this._ctor();
  }
}

// Create class object with String type
// - Create class object with type String
// - Pass an arrow function (that returns a new String object) to the class constructor
const FooString: Foo<String> = new Foo<String>(() => new String("foo"));

// Optional check.
// - Call alloc() of new class object to get a new object and
// - call the object's builtin toString() method to display it.
console.log(FooString.alloc().toString());
