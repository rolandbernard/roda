// test: Test moving around int type

fn fma(a: int, b: int, c: int): int {
    return a * b + c;
}

pub fn main(): bool {
    let a: int = 42;
    let b = a;
    let c = 12;
    let d = fma(a, b, c);
    return d != 1776;
}
