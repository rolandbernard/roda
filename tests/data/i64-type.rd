// test: Test moving around i64 type

fn fma(a: i64, b: i64, c: i64): i64 {
    return a * b + c;
}

pub fn main(): bool {
    let a: i64 = 42;
    let b = a;
    let c = 12;
    let d = fma(a, b, c);
    return d != 1776;
}
