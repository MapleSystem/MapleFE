class Klass {
  f1: number = 123;
  f2: string = "abc";
}
const obj = {
  func: (o: Klass) : Klass => {
    return <any> { ...o, private: false };
  },
};
console.log(obj.func(new Klass()));
