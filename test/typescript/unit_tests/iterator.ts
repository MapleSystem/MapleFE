interface Iface {
    name: string;
}

const myIter: Iterator<Iface> = Object.freeze({
  next () {
    return { done: true, value: undefined, };
  },
});


const iter: Iterable<Iface> = Object.freeze({
    [Symbol.iterator] () { return myIter; },
});
