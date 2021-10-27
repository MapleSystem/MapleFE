type NT<T, R> = ((args: Array<T>) => PromiseLike<Array<R>>) | ((args: Array<T>) => Array<R>) | ((args: Array<T>) => void);

