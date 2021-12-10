class IterableClass {
  public elements: number[];

  constructor() {
    this.elements = [];
  }

  add(element:number) {
    this.elements.push(element);
  }

  * [Symbol.iterator]() {
    let element:number;
    for (element of this.elements) {
      yield element;
    }
  }
}

let obj = new IterableClass();
obj.add(123);
obj.add(456);
for (let e of obj) {
  console.log(e);
}
