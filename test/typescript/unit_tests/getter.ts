var obj = { name: "dummy" };
Object.defineProperty(obj, "name", { get() { return "Name"; } });
console.log(obj.name);
