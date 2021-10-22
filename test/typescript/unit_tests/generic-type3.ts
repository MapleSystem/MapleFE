interface IFace {
    func<T>(): Promise<T extends PromiseLike<infer U> ? U : T>;
}
