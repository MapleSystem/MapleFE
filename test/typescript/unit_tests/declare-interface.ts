declare interface IFace<T> {
  readonly n: number;
  [index: number]: T;
}
