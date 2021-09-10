let prop: string = "foo";

class Klass {
  [key: string]: () => void;
  public [prop]() {
    console.log(prop);
  }
}

var obj: Klass = new Klass();
obj[prop]();
