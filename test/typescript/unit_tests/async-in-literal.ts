function func() {
  return {
    async message() {
      return "done";
    },
  };
}

console.log(func().message());
