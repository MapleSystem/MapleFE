let prop: string = "foo";

class Klass {
  public [prop]() { console.log("prop =", prop); }
  public bar()    { console.log("Function bar()"); }
}

var obj: Klass = new Klass();
obj[prop]();      // prop = foo
obj["foo"]();     // prop = foo

prop = "bar";
var obj2: Klass = new Klass();
obj2[prop]();     // Function bar()
obj2["foo"]();    // prop = bar
