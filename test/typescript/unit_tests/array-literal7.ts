const arr: (() => void)[] = [
  () => {
    console.log("Lambda0");
  },
  () => {
    console.log("Lambda1");
  },
];
console.log(arr);
arr[0]();
arr[1]();
