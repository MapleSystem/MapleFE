type U = { n: number };
type E<T> = T extends Array<infer U> ? T : U;
