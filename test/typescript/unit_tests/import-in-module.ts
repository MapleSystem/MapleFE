declare module 'M1' {
  export interface X {}
}

declare module 'M2' {
    import { X } from "M1";
    export namespace NS {
    }
}
