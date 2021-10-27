export declare type NT<T> = {
    [K in keyof T]: NonNullable<T[K]> extends boolean ? K : never;
  } extends { [_ in keyof T]: infer U; } ? U : never;
