export function func(arg: {let: Function | null}): void {
  if (!arg.let) {
    console.log("null");
  } else {
    arg.let();
  }
}

func({let: null});
func({let: () => { console.log("calling let()"); }});

export function func1(arg: {var: Function | null}): void {
  if (!arg.var) {
    console.log("null");
  } else {
    arg.var();
  }
}

func1({var: null});
func1({var: () => { console.log("calling var()"); }});

export function func2(arg: {break: Function | null}): void {
  if (!arg.break) {
    console.log("null");
  } else {
    arg.break();
  }
}

func2({break: null});
func2({break: () => { console.log("calling break()"); }});
