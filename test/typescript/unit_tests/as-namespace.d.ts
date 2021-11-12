// Proposal: export as namespace for UMD module output
// https://github.com/microsoft/TypeScript/issues/26532

// https://www.typescriptlang.org/docs/handbook/modules.html#umd-modules
export var x: number;
export as namespace NS;
