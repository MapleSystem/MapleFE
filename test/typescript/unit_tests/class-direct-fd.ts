class Foo {
  public f1: number = 0;
  public f2: number = 0;
  constructor(a: number, b: number) {
    this.f1 = a;
    this.f2 = b;
  }
}

var bar : Foo = {f1:1, f2:2};
bar.f1    = 3;            // direct dot
bar["f1"] = 4;            // direct prop
bar["p1"] = 10;           // prop

bar.f2    = bar.f1;       // direct dot = direct dot
bar.f2    = bar["f1"];    // direct dot = direct prop
bar["f1"] = bar["f2"];    // direct prop = direct prop
bar["f1"] = bar["p1"];    // direct prop = prop

bar["p2"] = bar["f2"];    // prop = direct prop
bar["p2"] = bar["p1"];    // prop = prop
bar["p2"] = bar["p2"] >>  bar["p1"];  // (int32_t)(xxx)  = yyy >> zzz
bar["p2"] = bar["p2"] >>> bar["p1"];  // (uint32_t)(xxx) = yyy >> zzz


console.log(bar.f1);
console.log(bar.f2);
console.log(bar["f1"]);
console.log(bar["f2"]);
console.log(bar["p1"]);
console.log(bar["p2"]);

