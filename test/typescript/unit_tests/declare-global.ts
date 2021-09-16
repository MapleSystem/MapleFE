// declare a variable in the global scope
declare global {
  interface IFace { name: string; }
}
export var obj: IFace = { name: "abc" };
