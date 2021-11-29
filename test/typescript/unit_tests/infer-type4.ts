type MyType<K extends string[]> = K extends [infer U, ...infer V] ? Extract<V, string[]> : K;
