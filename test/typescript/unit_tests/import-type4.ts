declare const obj: {
  readonly 'X.Foo': Record<string, import("./export-type").Foo>;
};
export { obj };
