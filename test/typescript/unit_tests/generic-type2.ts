class Klass {
  [x: string]: number;
}

class EXT<B, T> {
  [key: string]: B;
}

type TYPE<B, T> = EXT<B, T>[keyof Klass];
