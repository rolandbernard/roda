// test: Test moving around u32 type

fn fma(a: u32, b: u32, c: u32): u32 {
    return a * b + c;
}

pub fn main(): bool {
    let a: u32 = 42;
    let b = a;
    let c = 12;
    let d = fma(a, b, c);
    return d != 1776;
}
