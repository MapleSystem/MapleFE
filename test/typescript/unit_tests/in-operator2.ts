type T = {
  name: string;
  age: number;
};

var obj: T = { name: "John", age: 30 };
console.log("name" in obj);
console.log("Age" in obj);
