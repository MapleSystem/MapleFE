class A {
  id: number;
}

class B {
  flag: false;
}

class C {
  name: string;
}

type I = A & (B["flag"] extends true ? B : C);
let v: I;
v!.id = 10;
v!.name = "B";

console.log(v!.id, v!.name);
