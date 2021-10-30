interface IFace {
  num: number;
}

interface IFace2<
T extends null | IFace,
> extends IFace {
  prop: T;
}
