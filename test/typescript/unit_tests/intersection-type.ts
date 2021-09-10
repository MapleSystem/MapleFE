interface A {
  id: number;
}

interface B {
  name: string;
}

type I = A & B;
let v: I;
v!.id = 10;
v!.name = "B";

console.log(v!.id, v!.name);
