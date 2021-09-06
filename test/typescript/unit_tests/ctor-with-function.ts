//const obj = new Pool<IAdditiveLightPass>(() => ({ subModel: null!, passIdx: -1, dynamicOffsets: [], lights: [] }), 16);
class Klass {
  func: () => any;
  num: number;
  constructor(f, n) {
    this.func = f;
    this.num = n;
  }
}
const obj = new Klass(() => ({ n: 1 }), 16);
console.log(obj);
