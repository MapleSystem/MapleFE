declare type Type<T> = { [K in keyof T]: K; }[keyof T] & string;
