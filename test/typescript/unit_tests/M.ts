var x: number = 2;
export { x as default }; // ref
//export default x;      // copy
export function getx(): number {
  return x;
}
export function setx(v): void {
  x = v;
}
