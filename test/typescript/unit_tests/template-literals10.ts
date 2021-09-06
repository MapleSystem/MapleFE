var flag = false;
var a = ".x";
console.log(
  `${`if(typeof ${flag ? "(prop)" : "prop"}!=="object"){` + "o"}${a}=prop;` +
    `}`
);
