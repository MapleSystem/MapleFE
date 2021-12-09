type Names<T> = { [K in keyof T]: T[K] extends (...args: Array<any>) => any ? K : never; }[keyof T] & string;
type MyType<T extends {}, M extends Names<Required<T>>> = Required<T>[M] extends (...args: any[]) => any ? string : number;
