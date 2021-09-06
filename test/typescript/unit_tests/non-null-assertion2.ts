function foo(): Object | null {
  return new Object();
}

export const obj = (() => {
  return foo()!;
})();

console.log(obj);
