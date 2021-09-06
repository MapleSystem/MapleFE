export type X<A extends boolean = false> = A extends true ? any[] : string;

var x: X<true> = [1, 2, 3];
var y: X = "abc";
console.log(x, y);
