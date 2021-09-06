class Klass {
  arr = [{ list: { f1: 0, f2: 1 } }];
}
var obj: Klass = new Klass();
var list2 = true ? { ...obj.arr[0] } : obj.arr[0].list;
console.log(list2);
