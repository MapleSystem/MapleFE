var s = "abc de fg";
s = s.replace(/[^A-Za-z0-9\+\/\=]/g, "");
console.log(s);
