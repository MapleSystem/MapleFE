const prop = "flag";

class Klass {
  public [prop]?: string = "example";
}

var obj: Klass = new Klass();
console.log(obj[prop]);
