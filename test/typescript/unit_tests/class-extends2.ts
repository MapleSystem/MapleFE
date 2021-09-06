class Car {
  private _make: string;
  constructor(make: string) {
    this._make = make;
  }
  public getMake(): string {
    return this._make;
  }
}

class Model extends Car {
  private _model: string;
  constructor(make: string, model: string) {
    super(make);
    this._model = super.getMake() + model;
  }
  public getModel(): string {
    return this._model;
  }
}

let passat: Model = new Model("VW", "Passat");
console.log(passat.getMake(), passat.getModel());
