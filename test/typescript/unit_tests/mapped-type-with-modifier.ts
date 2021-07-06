// ref: cocos splash-screen.ts line 59.
// ref: https://www.typescriptlang.org/docs/handbook/2/mapped-types.html#mapping-modifiers

type mutableType<T> = {
  -readonly [prop in keyof T]: T[prop];
};
