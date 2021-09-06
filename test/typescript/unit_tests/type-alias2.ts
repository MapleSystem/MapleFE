type UT<T, I extends number> = {
  base: T;
  ext: T extends ReadonlyArray<infer U> ? UT<U, [-1, 0, 1, 2][I]> : T;
}[I extends -1 ? "base" : "ext"];
