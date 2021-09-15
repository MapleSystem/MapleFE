interface IFace {
    any<T>(arg: (T | Iterable<T>)[] | string): Iterable<T>;
}
