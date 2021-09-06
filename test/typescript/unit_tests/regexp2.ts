function repl(s: string) {
  return s.replace(/\s\S/g, ".");
}
console.log(repl("abc def gh"));
