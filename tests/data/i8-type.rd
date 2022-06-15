// test: Test moving around i8 type

fn fma(a: i8, b: i8, c: i8): i8 {
    return a * b + c;
}

pub fn main(): bool {
    let a: i8 = 42;
    let b = a;
    let c = 12;
    let d = fma(a, b, c);
    return d != -16;
}
