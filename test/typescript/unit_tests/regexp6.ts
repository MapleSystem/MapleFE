const str = "foo bar tee";
let res: null | any[] = /Foo/.exec(str) || /bar/.exec(str);
console.log(res);
