var str: string = "abc/ab/1234abcdef";
var re = /.*[/\\][0-9a-fA-F]{2}[/\\]([0-9a-fA-F-@]{8,}).*/;
console.log(str);
str = str.replace(re, "replaced");
console.log(str);
