const obj = {
  prop: "foo",
  get getProp(): string {
    return this.prop;
  },
  set setProp(newVal: string) {
    this.prop = newVal;
  },
};

console.log(obj.getProp);
obj.setProp = "bar";
console.log(obj.getProp);
