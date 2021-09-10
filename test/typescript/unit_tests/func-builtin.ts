interface Employee {
  firstName: string;
  lastName: string;
}

let john: Employee = {
  firstName: "John",
  lastName: "Smith",
};

let jane: Employee = {
  firstName: "Jane",
  lastName: "Doe",
};

var func;
var res;

function fullName(this: Employee): string {
  return this.firstName + " " + this.lastName;
}

// call() and apply() returns the result of calling the function directly with the obj instance arg
res = fullName.call(john);
console.log("Name: " + res); // output: "Name: John Smith"
res = fullName.call(jane);
console.log("Name: " + res); // output: "Name: Jane Doe"
res = fullName.apply(john);
console.log("Name: " + res); // output: "Name: John Smith"
res = fullName.apply(jane);
console.log("Name: " + res); // output: "Name: Jane Doe"

// bind() returns a copy of the function bound to the obj instance arg
func = fullName.bind(john);
console.log("Name: " + func()); // output: "Name: John Smith"
func = fullName.bind(jane);
console.log("Name: " + func()); // output: "Name: Jane Doe"
