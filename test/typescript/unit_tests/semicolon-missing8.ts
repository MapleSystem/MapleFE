function func(f: Function): void {
  f();
}

func((e:any) => {
  if (e) throw e
  console.log('OK')
})
