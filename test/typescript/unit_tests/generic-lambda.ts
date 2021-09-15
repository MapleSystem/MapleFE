const func = (() => (<T>(x: any): x is T[] => x && typeof x.length === 'number'))();
console.log(func([1,2]));
