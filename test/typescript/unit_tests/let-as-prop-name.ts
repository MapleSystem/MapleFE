export function func(arg: {let: Function | null}): void {
  if (!arg.let) {
    console.log("null");
  } else {
    arg.let();
  }
}

func({let: null});
func({let: () => { console.log("calling let()"); }});
