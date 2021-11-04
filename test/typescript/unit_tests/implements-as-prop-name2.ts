interface IFace {
  implements? : string;
}

var obj: IFace = { implements: "implements" };
console.log(obj);
