class VType {}
class V2 extends VType {s: string = "abc"}
class V3 extends VType {n: number = 123}

const BuiltinVTypes: typeof VType[] = [V2, V3];
