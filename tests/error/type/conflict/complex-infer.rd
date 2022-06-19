// test: Test moving around structs
// stderr: = error[E0008]: 13:12-34: type error, conflicting types `u8` and `[1]_`\n

fn foo(): u8 {
    let h;
    let g;
    let d;
    let f = [d];
    let e = [f[0]];
    let b = (a = g, b = e[0], c = e[1]);
    let a = (a = b.c, b = [e[0] + g]);
    let c = (a = b, b = a.b[1]);
    return [[[[c.b]]], h][0][0][0];
}

pub fn main(): int {
    foo(); // Return value of this is undefined
    return 0;
}

