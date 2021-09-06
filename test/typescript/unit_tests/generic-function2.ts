class Klass {
  func<T extends (...any) => void>(cb: T): typeof cb {
    return cb;
  }
}
