interface IFace {
  name: string;
}

interface ICtor {
  new(name: string);
}

class Car implements IFace {
  name: string;
  constructor(n: string) {
    this.name = n;
  }
}

function carFactory(myClass: ICtor, name: string) {
  return new myClass(name);
}

let car = carFactory(Car, "myCar");
console.log(car);
