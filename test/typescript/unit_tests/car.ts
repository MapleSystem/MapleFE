class Vehicle {
  name: string;
  constructor(name: string) {
    this.name = name;
  }
}
class Car extends Vehicle {
  constructor(name: string) {
    super(name);
  }
}
let car: Car = new Car("A car");
console.log(car.name);
