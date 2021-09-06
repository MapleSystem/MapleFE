function class_deco(ctor): void {
  console.log("Class constructor is :", ctor);
}

@class_deco
export class Foo {
  readonly foo_var: number = 1;
}
