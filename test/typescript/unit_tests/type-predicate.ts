interface Foo {
    foo: number;
}

interface Bar {
    bar: number;
}

function isFoo(arg: any): arg is Foo {
    return arg.foo !== undefined;
}

// user defined type guard
function doStuff(arg: Foo | Bar) {
    if (isFoo(arg)) {
        console.log(arg.foo);
    }
    else {
        console.log(arg.bar);
    }
}

doStuff({ foo: 123 });
doStuff({ bar: 123 });
