async function func() {
  return "done";
}

(async () => {
    const val = await func();
    console.log(val);
})();
