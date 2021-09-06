class Klass {
  items: number[] = [];

  func(num: number) {
    let i, list;
    for (i = 0, list = this.items; i < num; i++) console.log(i, list[i]);
  }
}

var obj: Klass = new Klass();
obj.items.push(1, 2, 3, 4, 5, 6);
obj.func(6);
