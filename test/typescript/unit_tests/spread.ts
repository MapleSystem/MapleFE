const arr: number[] = [1, 2, 3];
var arr2 = [ 0, ...arr ];
console.log(arr2);

function func() {
 return { f1: 123, f2: "abc" };
}
var obj2 = { f0: "OK", ...func() };
console.log(obj2);
