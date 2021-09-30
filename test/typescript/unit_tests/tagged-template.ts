// Tagged Template Literal
function bar(a: TemplateStringsArray, b:number, c:string) {
  return "bar: a=[" + a + "] b=[" + b + "] c=[" + c + "]";
}
function f() {
  let x: number = 10;
  let y: string = "abc";
  return bar`PX
  ${x}px${y}PX
  `;
}
console.log(f());
