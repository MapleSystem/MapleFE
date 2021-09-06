for (var i = 3; i >= 0; --i) {
  try {
    if (i == 0) throw new Error("Zero");
    console.log("try", 3 / i);
  } catch (err) {
    console.log("catch", err);
  } finally {
    console.log("finally", i);
  }
}
