type Concrete<T> = {
    [P in keyof T]-?: T[P];
};

interface IFace {
  name: string;
  age?: number;
}

class Klass implements Concrete<IFace> {
  name: string = "No-name";
  age: number = 0;
}

var obj: Klass = new Klass();
console.log(obj);
