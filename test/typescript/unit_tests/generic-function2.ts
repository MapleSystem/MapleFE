class Klass {
  func<T extends (...any: any) => void>(cb: T): typeof cb {
    return cb;
  }
}
