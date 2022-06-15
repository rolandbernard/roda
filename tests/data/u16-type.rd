// test: Test moving around u16 type

fn fma(a: u16, b: u16, c: u16): u16 {
    return a * b + c;
}

pub fn main(): bool {
    let a: u16 = 42;
    let b = a;
    let c = 12;
    let d = fma(a, b, c);
    return d != 1776;
}
