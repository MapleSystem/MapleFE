let funcs: (() => void)[] = [];
function initialize() {
  var msgs: [string, string] = ["Hello", "World"];
  for (var i = 0; i < msgs.length; i++) {
    let msg: string = msgs[i];
    funcs[i] = () => console.log(msg);
  }
}
initialize();
for (var i = 0; i < funcs.length; i++) {
  funcs[i]();
}
