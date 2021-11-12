class A {
  id: number = 0;
}

class B {
  flag: false = false;
}

class C {
  name: string = "";
}

type I = A & (B["flag"] extends true ? B : C);
let v: I = {id: 0, name: ""};
v!.id = 10;
v!.name = "B";

console.log(v!.id, v!.name);
