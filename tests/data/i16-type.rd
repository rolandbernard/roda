// test: Test moving around i16 type

fn fma(a: i16, b: i16, c: i16): i16 {
    return a * b + c;
}

pub fn main(): bool {
    let a: i16 = 42;
    let b = a;
    let c = 12;
    let d = fma(a, b, c);
    return d != 1776;
}
