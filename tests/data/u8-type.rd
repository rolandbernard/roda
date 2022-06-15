// test: Test moving around u8 type

fn fma(a: u8, b: u8, c: u8): u8 {
    return a * b + c;
}

pub fn main(): bool {
    let a: u8 = 42;
    let b = a;
    let c = 12;
    let d = fma(a, b, c);
    return d != 240;
}
