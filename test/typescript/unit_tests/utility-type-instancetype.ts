class Klass {
  x: number = 33;
  public getNum() {
    return this.x >> 2;
  }
}

class Klass2 {
  map: Map<number, InstanceType<typeof Klass>> | undefined = undefined;
}
