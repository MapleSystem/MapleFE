class Klass {
  prop: string = "";
  get getProp(): string {
    return this.prop;
  }
  init(val: string) {
    this.prop = val;
  }
  set setProp(newVal: string) {
    this.init(newVal);
  }
}

var obj: Klass = new Klass();
console.log(obj.getProp);
obj.setProp = "bar";
console.log(obj.getProp);
