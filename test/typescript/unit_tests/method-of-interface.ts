interface Klass1 {}

interface Klass2 {
  name: string;
}

interface Klass3 {
  getObj(o: Klass1): Klass2;
}
