class Klass {
  u: number = 0;
  v: string = "";

  constructor ({u, v}: Partial<Klass>= {}) {
    this.u = u ? u : this.u;
    this.v = v ? v : this.v;
  }
}
