function func<T>(e: T) : Array<T> {
  return [e];
}
function foo<T>(arg: any) {
  return func<T>(arg)[0] as T;
}
