type Type<T extends abstract new (...args: any) => any> = T extends abstract new (...args: infer P) => any ? P : never;

