// object constructor property used in cocos-creator class.ts
function func() {}
let f = new func();
console.log(f.constructor);
f.constructor.prop1 = 1;
Object.defineProperty(f.constructor, "prop2", {
  value: 2,
  writable: true,
  enumerable: true,
});
console.log(f.constructor);
