var obj = { x: "abc", name: "" };

Object.defineProperty(obj, "name", {
  get(this) {
    console.log("Return obj.x");
    return this["x"];
  },
  set(this, val: any) {
    console.log(`Set obj.x to '${val}'`);
    this["x"] = val;
  },
  enumerable: false,
});

obj.name = "def";
console.log(obj, obj.name);
