// #private; for ECMAScript’s pound (#) private fields
// https://www.typescriptlang.org/docs/handbook/2/classes.html#caveats
// https://github.com/microsoft/TypeScript/issues/38050
declare class Klass {
    #private;
    constructor();
}
