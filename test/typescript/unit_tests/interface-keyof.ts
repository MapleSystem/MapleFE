interface Interf<T> {
  name: keyof T;
  val: T[keyof T];
}
