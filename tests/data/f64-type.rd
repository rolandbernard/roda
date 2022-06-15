// test: Test moving around f64 type

fn foo(a: f64, b: f64, c: f64): f64 {
    return a * b * c;
}

pub fn main(): bool {
    let a: f64 = 25e-1;
    let b = a;
    let c = 543.25;
    let d = foo(a, b, c);
    return d != 3395.3125;
}
