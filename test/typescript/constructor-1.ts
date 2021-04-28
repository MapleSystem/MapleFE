// Constructor function for Person objects
function Person(first, last, age, eye) {
  this.firstName = first;
  this.lastName = last;
  this.age = age;
  this.eyeColor = eye;
}

//// Create a Person object
var myFather = new Person("John", "Doe", 50, "blue");
console.log(myFather.age);
