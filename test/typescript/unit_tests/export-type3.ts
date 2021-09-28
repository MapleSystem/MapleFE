export type Type<B, K extends keyof B = keyof B>
= Pick<B, Exclude<keyof B, K>> 
& Partial<Pick<B, K>> extends infer U ? {[KT in keyof U]: U[KT]} : never;

