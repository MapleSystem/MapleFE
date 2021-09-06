// tsc -t es6
interface IFace {
  create<T = any>(
    args: Iterable<readonly [PropertyKey, T]>
  ): { [k: string]: T };
  create(args: Iterable<readonly any[]>): any;
}
