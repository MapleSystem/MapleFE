function func<T>(f: Function) {
  return <() => Array<T>>f;
}
