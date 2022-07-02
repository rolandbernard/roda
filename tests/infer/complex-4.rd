// test: Test moving around structs

fn foo(): u8 {
    let h;
    let g;
    let d;
    let f = [d];
    let e = [f[0]];
    let b = (g, e[0], e[1]);
    let a = (b.2, [e[0] + g]);
    let c = (b, a.1[1]);
    return [[[[c.1]]], h][0][0][0][0];
}

pub fn main(): int {
    foo(); // Return value of this is undefined
    return 0;
}

