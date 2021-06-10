// https://www.typescriptlang.org/docs/handbook/release-notes/typescript-2-0.html#non-null-assertion-operator
// used in cocos director.ts
function processEntity(e?: any) {
  let s = e!.name; // Assert that e is non-null and access name
  let t = e.name!;
}
