interface IFace {
  export() : string;
}

var obj: IFace = { export: () => "Export" };
console.log(obj);
