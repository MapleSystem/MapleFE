class Klass {
  func: () => any;
  num: number;
  constructor(f: () => any, n: number) {
    this.func = f;
    this.num = n;
  }
}
const obj = new Klass(() => { n: 1 }, 16);
console.log(obj);
