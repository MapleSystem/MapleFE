function foo(): Object | null {
  return new Array();
}

export const obj = (() => {
  return foo()! as unknown as Array<number>;
})();

console.log(obj);
