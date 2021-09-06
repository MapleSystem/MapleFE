let prop: string = "foo";

class Klass {
  public [prop]() {
    console.log(prop);
  }
}

var obj: Klass = new Klass();
obj[prop]();
