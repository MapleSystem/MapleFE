"use strict";
function func(x) {
    if (typeof x === "number")
        return x * x;
    return undefined;
}
let v = func("a");
console.log(v);
v = func(3);
console.log(v);
