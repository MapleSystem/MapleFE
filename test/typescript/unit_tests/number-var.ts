var func: (number: number) => string;
function conv(number: number): string {
  return number as unknown as string;
}

func = conv;
console.log(func(123));
