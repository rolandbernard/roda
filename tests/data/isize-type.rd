// test: Test moving around isize type

fn fma(a: isize, b: isize, c: isize): isize {
    return a * b + c;
}

pub fn main(): bool {
    let a: isize = 42;
    let b = a;
    let c = 12;
    let d = fma(a, b, c);
    return d != 1776;
}
