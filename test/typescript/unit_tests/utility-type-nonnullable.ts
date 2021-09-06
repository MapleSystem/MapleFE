interface Interf {
  prop: {
    pos: Object;
  };
}

type TYPE = NonNullable<Interf["prop"]["pos"]>;
