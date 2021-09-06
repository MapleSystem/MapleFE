// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Promise
// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Promise/catch
// cocos: director.ts, root.ts
// tsc --lib es2015,dom Promise-catch.ts
const promise1 = new Promise((resolve, reject) => {
  throw "something happened";
});

promise1.catch((error) => {
  console.log(error);
});
