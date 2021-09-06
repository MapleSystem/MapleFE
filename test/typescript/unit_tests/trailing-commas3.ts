class Klass {}
function func(m?: Klass | number) {
  return m;
}

var obj: Klass = new Klass();
console.log(func(obj));
