class VType {}
class V2 extends VType {s: string}
class V3 extends VType {n: number}

const BuiltinVTypes: typeof VType[] = [V2, V3];
