// Extracted from https://github.com/cocos-creator/engine/blob/develop/cocos/core/gfx/pipeline-state.jsb.ts#L33
export declare type NT<T> = {
  [P in keyof T]: T[P] extends Record<string, infer U> ? Exclude<U, string> | keyof T[P] : T[P];
};
