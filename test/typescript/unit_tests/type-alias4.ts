type MyType<T> = T extends string ? T : string;
type UT<T> = MyType<T> extends string ? string : number;
