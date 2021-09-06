class Klass {
  protected n: Map<string, (...args: any[any]) => void> = new Map();
}

var obj: Klass = new Klass();
console.log(obj);
