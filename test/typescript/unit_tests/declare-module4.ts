export var x: number;
declare global {
    module Module {
        interface IFace {
            expect: string;
        }
    }
}
