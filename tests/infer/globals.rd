// test: Test moving around structs

let h;
let g;
let d;

fn foo(): u8 {
    let f = [d];
    let e = [f[0]];
    let b = (a = g, b = e[0], c = e[1]);
    let a = (a = b.c, b = [e[0] + g]);
    let c = (a = b, b = a.b[1]);
    return [[[[c.b]]], h][0][0][0][0];
}

pub fn main(): int {
    foo(); // Return value of this is undefined
    return 0;
}

