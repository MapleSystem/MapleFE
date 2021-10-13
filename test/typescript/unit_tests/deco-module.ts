export function prop_deco(msg: string) {
  return function (target: any, name: string) {
    console.log("Accessed", name, msg, target);
  };
}
