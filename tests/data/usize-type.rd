// test: Test moving around usize type

fn fma(a: usize, b: usize, c: usize): usize {
    return a * b + c;
}

pub fn main(): bool {
    let a: usize = 42;
    let b = a;
    let c = 12;
    let d = fma(a, b, c);
    return d != 1776;
}
