// test: Test moving around uint type

fn fma(a: uint, b: uint, c: uint): uint {
    return a * b + c;
}

pub fn main(): bool {
    let a: uint = 42;
    let b = a;
    let c = 12;
    let d = fma(a, b, c);
    return d != 1776;
}
