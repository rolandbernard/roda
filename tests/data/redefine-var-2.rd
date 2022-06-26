// test: Test shadowing of variables in same scope

fn fma(a: i32, b: i32, c: i32): i32 {
    let a = (2 * a) as i64;
    let a = (3 * a) as f64;
    let a = (4.5 * a) as i32;
    return a * b + c;
}

pub fn main(): bool {
    let a: i64 = 42;
    let a = a as i32;
    let c = 12;
    let d = fma(a, a, c);
    return d != 47640;
}
