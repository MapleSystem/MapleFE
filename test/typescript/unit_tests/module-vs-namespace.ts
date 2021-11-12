module m_for_in {
    let arr : Array<number> = [12, 34, 56];
    for (let x in arr) {
        console.log(x, arr[x]);
    }
}
module m_for_of {
    let arr : Array<number> = [11, 22, 33];
    for (let x of arr) {
        console.log(x);
    }
}
namespace ns_for_in {
    let arr : Array<number> = [12, 34, 56];
    for (let x in arr) {
        console.log(x, arr[x]);
    }
}
namespace ns_for_of {
    let arr : Array<number> = [11, 22, 33];
    for (let x of arr) {
        console.log(x);
    }
}
