var x: number = 2;
var y: number = 4;
var z: number = 6;

let px = `${x + y}px and ${z ? x + z : y + z}px`;
console.log(px);
