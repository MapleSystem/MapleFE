class Klass {
  list = { f1: 0, f2: 1 };
}
var obj: Klass = new Klass();
var list2 = true ? { ...obj.list } : obj.list;
console.log(list2);
