class Klass {
  u: number;
  v: string;

  constructor ({u, v}: Partial<Klass>= {}) {
    this.u = u ? u : this.u;
    this.v = v ? v : this.v;
  }
}
