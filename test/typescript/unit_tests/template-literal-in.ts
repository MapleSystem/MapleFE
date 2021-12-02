type MyType<B extends Record<string|number, any>> = {
  [K in`${Extract<keyof B, string|number>}`]: B[K]
};
