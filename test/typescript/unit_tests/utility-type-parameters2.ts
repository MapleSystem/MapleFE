function func(n: number, s: string): string {
  return n + s;
}

function wrapper(
  n: Parameters<typeof func>[0],
  s: Parameters<typeof func>[1] | undefined
): string {
  return func(n, s!);
}

console.log(wrapper(123, "abc"));
