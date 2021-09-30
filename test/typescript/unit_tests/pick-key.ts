interface IFace {
  s: string;
  n: number;
}
declare type Type = { [K in keyof Pick<IFace, "s">]: true; };

