function func(name: string) {
  return Function("target", `${"try {\n" + "  target."}${name}();\n` + `}`);
}
