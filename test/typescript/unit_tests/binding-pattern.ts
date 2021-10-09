interface IFace {
    Num: number;
    Str: string;
}

declare const func: ({ Num: num, Str: str }?: IFace) => boolean;
export { IFace, func };
