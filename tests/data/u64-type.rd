// test: Test moving around u64 type

fn fma(a: u64, b: u64, c: u64): u64 {
    return a * b + c;
}

pub fn main(): bool {
    let a: u64 = 42;
    let b = a;
    let c = 12;
    let d = fma(a, b, c);
    return d != 1776;
}
