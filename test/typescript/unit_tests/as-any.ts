function func() {
  return { list: [1, 2, 3] };
}
let s = func().list as any;
console.log(s);
