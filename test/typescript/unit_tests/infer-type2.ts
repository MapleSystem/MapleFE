// Extracted from https://github.com/cocos-creator/engine/blob/develop/cocos/core/gfx/pipeline-state.jsb.ts#L33
declare type RecursivePartial<T> = {
  [P in keyof T]?: T[P] extends Array<infer U>
    ? Array<RecursivePartial<U>>
    : T[P] extends ReadonlyArray<infer V>
    ? ReadonlyArray<RecursivePartial<V>>
    : RecursivePartial<T[P]>;
};
