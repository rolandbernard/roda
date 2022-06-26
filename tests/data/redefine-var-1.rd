// test: Test shadowing of variables in same scope

fn fma(a: i32, b: i32, c: i32): i32 {
    let a = 2 * a;
    let a = 3 * a;
    let a = 4 * a;
    return a * b + c;
}

pub fn main(): bool {
    let a = 42;
    let a = a;
    let c = 12;
    let d = fma(a, a, c);
    return d != 42348;
}
