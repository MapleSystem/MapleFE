class Klass {
  str: string = "no";
}
function func(obj: Klass) {
  if (obj.str === "yes") {
    return obj; 
  } else {
    try {
      throw "Exception";
    } catch (e: unknown) {
      console.log(e);
      return obj;
    }
  }
}

console.log(func(new Klass()));
