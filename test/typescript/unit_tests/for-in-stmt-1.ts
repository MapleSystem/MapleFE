class Klass {
  [key: string]: number | string;
  x: number = 0;
  s: string = "";
}

var obj: Klass = { x: 1, s: "123" };
for (const k in obj) {
  console.log(k, obj[k]);
}
