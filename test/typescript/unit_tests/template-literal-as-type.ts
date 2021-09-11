const X = "X";
const T: `${typeof X}type` = `${X}type`;
console.log(typeof T);
console.log(T);
