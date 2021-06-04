// Javascript optional chaining operator. Used in Cocos Ceator class.ts
// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Operators/Optional_chaining
// Requires nodejs v.14.0.0 and above.
// https://www.digitalocean.com/community/tutorials/how-to-install-node-js-on-ubuntu-20-04

let obj1 = {};  // has hasOwnProperty prop
let obj2;       // does not have hasOwnProperty prop
console.log(obj1.hasOwnProperty('prop1'));
console.log(obj2?.hasOwnProperty('prop1'));

