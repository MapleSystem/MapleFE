var v: boolean = (() => {
  const buf = new ArrayBuffer(2);
  new DataView(buf).setInt16(0, 256, true);
  return new Int16Array(buf)[0] === 256;
})();
console.log(v);
