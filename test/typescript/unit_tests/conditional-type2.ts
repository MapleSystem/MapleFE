type Type<B, T> = { [K in keyof B]: B[K] extends T ? never : K };
