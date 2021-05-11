// compile with tsc -t ES2015
declare namespace ns_a {
    export var foo: number;
}

declare namespace ns_b {
    export var foo: number;
}

export {ns_a as ns_1};
export {ns_b as ns_2};
export {ns_a, ns_b as ns_ab};
