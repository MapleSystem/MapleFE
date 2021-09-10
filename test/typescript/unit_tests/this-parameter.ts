class Klass {
  [key: string]: Function | number;
}

var func = (obj: Klass) => {
  const desc = Object.getOwnPropertyDescriptor(obj, "f");
  console.log(desc);
  if (typeof desc!.value === "function") {
    const v = desc!.value;
    obj["f"] = function (this) {
      console.log("Calling the new function");
      return v.call(this, ...arguments);
    };
  }
};

var o = {
  n: 123,
  f: function () {
    console.log("Calling f()");
    return this;
  },
};

func(o);
console.log(o.f());
