// test: Test moving around f32 type

fn foo(a: f32, b: f32, c: f32): f32 {
    return a * b * c;
}

pub fn main(): bool {
    let a: f32 = 25e-1;
    let b = a;
    let c = 543.25;
    let d = foo(a, b, c);
    return d != 3395.3125;
}
