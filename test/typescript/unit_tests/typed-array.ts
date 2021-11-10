let buf = new ArrayBuffer(16);
let bytes = new Uint8Array(buf);
for (let i = 0; i < bytes.length; i++)
  bytes[i] = i * 15;

console.log("\nInt8Array", Int8Array);
let Int8View = new Int8Array(buf);
for (let i = 0; i < Int8View.length; i++)
  console.log(i, Int8View[i]);
console.log("\nUint8Array", Uint8Array);
let Uint8View = new Uint8Array(buf);
for (let i = 0; i < Uint8View.length; i++)
  console.log(i, Uint8View[i]);
console.log("\nUint8ClampedArray", Uint8ClampedArray);
let Uint8ClampedView = new Uint8ClampedArray(buf);
for (let i = 0; i < Uint8ClampedView.length; i++)
  console.log(i, Uint8ClampedView[i]);
console.log("\nInt16Array", Int16Array);
let Int16View = new Int16Array(buf);
for (let i = 0; i < Int16View.length; i++)
  console.log(i, Int16View[i]);
console.log("\nUint16Array", Uint16Array);
let Uint16View = new Uint16Array(buf);
for (let i = 0; i < Uint16View.length; i++)
  console.log(i, Uint16View[i]);
console.log("\nInt32Array", Int32Array);
let Int32View = new Int32Array(buf);
for (let i = 0; i < Int32View.length; i++)
  console.log(i, Int32View[i]);
console.log("\nUint32Array", Uint32Array);
let Uint32View = new Uint32Array(buf);
for (let i = 0; i < Uint32View.length; i++)
  console.log(i, Uint32View[i]);
console.log("\nFloat32Array", Float32Array);
let Float32View = new Float32Array(buf);
for (let i = 0; i < Float32View.length; i++)
  console.log(i, Float32View[i]);
console.log("\nFloat64Array", Float64Array);
let Float64View = new Float64Array(buf);
for (let i = 0; i < Float64View.length; i++)
  console.log(i, Float64View[i]);
// console.log("\nBigInt64Array", BigInt64Array);
// let BigInt64View = new BigInt64Array(buf);
// for (let i = 0; i < BigInt64View.length; i++)
//   console.log(i, BigInt64View[i]);
// console.log("\nBigUint64Array", BigUint64Array);
// let BigUint64View = new BigUint64Array(buf);
// for (let i = 0; i < BigUint64View.length; i++)
//   console.log(i, BigUint64View[i]);
