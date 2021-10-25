// https://github.com/microsoft/TypeScript/pull/40336
type Split<S extends string, D extends string> =
    string extends S ? string[] :
    S extends '' ? [] :
    S extends `${infer T}${D}${infer U}` ? [T, ...Split<U, D>] :
    [S];

type T40 = Split<'foo', '.'>;          // ['foo']
type T41 = Split<'foo.bar.baz', '.'>;  // ['foo', 'bar', 'baz']
type T42 = Split<'foo.bar', ''>;       // ['f', 'o', 'o', '.', 'b', 'a', 'r']
type T43 = Split<any, '.'>;            // string[]
