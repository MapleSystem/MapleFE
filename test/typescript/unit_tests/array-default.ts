// the type of arr element is never by default
const arr: [] = [];
(arr as any[]).push([]);
arr.push.apply(arr, {} as any);
console.log(arr);
