// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Object/keys
// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Array/map
var objA = { objB: { objC: {} } };
const list = Object.keys(objA.objB.objC).map((x) => objA.objB.objC[x]);
