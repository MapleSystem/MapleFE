function func<T>(x: T): T {
  return x;
}
var list = { f1: 0, f2: 1 };
class Klass {
  public static fd = func({ ...list });
}
console.log(Klass.fd);
