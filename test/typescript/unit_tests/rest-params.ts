function func(msg: string, ...params: any[]) {
  console.log(msg, params);
}

func("Rest: ", 1, 2, 3, "abc", "def");
