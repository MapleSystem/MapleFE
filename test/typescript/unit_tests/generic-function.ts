class Klass {
  func<T extends (...any) => void>(cb: T): typeof cb {
    return cb;
  }
}

var obj: Klass = new Klass();
console.log(obj);
