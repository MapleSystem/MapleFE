class Vehicle {
  constructor (name) { this.name = name; }
}
class Car extends Vehicle {
  constructor (name) { super(name); }
}
class MyCar extends Car {
  constructor (name) { super(name); }
}

let car = new Car("Tesla");
let mycar = new MyCar("Tesla Model X");

for(g = 0; g < 2; ++g) { 
  let arr = []
  let objs =  [  arr,   Array,   Vehicle,   Car,   MyCar,   car,   mycar,   Function,   Object,  ];
  let names = [ "arr", "Array", "Vehicle", "Car", "MyCar", "car", "mycar", "Function", "Object", ];
  console.log("digraph JS" + g + " {\nnewrank=true;\nnodesep=1;\nranksep=1;");
  let num = objs.length;
  let k = num;
  for(let i = 0; i < num; ++i) {
    x = typeof objs[i].prototype;
    if(x === "function" || x === "object") {
      objs[k] = objs[i].prototype;
      names[k] = names[i] + "_prototype";
      console.log("subgraph cluster_" + names[i] + " {\nrank=same;\ncolor=white;\n" + names[i] + ";");
      console.log(names[k] + "[label=\"" + names[i] + ".prototype\", shape=box];\n }");
      console.log(names[i] + " -> " + names[k] + " [label=\"prototype\", color=blue, fontcolor=blue];");
      k++;
    }
  }
  num = objs.length;
  for(let i = 0; i < num; ++i) {
    for(let j = 0; j < num; ++j) {
      if(g > 0 && objs[i].constructor === objs[j])
        console.log(names[i] + " -> " + names[j] + " [label=\"ctor\", color=darkgreen, fontcolor=darkgreen];");
      if(objs[i].__proto__ === objs[j])
        console.log(names[i] + " -> " + names[j] + " [label=\"__proto__\", color=red, fontcolor=red];");
    }
  }
  console.log("}");
}
