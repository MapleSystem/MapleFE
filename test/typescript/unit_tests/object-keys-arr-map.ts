var objA  = {objB:{objC:{}}};
const list = Object.keys(objA.objB.objC).map((x) => objA.objB.objC[x]);
