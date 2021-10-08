function func(f: Function): void {
  f();
}

func((e) => {
  if (e) throw e
  console.log('OK')
})
