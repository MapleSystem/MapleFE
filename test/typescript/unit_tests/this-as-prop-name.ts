interface IFace {
  this: Object;
};

var obj: IFace = { this: {name: "Name"} };
console.log(obj);
console.log(obj["this"]);
