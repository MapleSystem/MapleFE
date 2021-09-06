class Klass {
  items: number[] = [];

  func() {
    let i, list, len;
    for (i = 0, list = this.items, len = this.items.length; i < len; i++)
      console.log(i, list[i]);
  }
}

var obj: Klass = new Klass();
obj.items.push(1, 2, 3, 4, 5, 6);
obj.func();
