interface Foo<T> {
  func<T>(callback: (this: void, v: T) => v is T, thisArg?: any): T;
}
