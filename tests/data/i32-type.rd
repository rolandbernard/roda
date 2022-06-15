// test: Test moving around i32 type

fn fma(a: i32, b: i32, c: i32): i32 {
    return a * b + c;
}

pub fn main(): bool {
    let a: i32 = 42;
    let b = a;
    let c = 12;
    let d = fma(a, b, c);
    return d != 1776;
}
