interface IFace {
  "func-name"() : string;
}

var obj: IFace = { "func-name": () => "Function name" };
console.log(obj);
