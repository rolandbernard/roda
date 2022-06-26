// test: Test shadowing of variables in child scope

fn fma(a: i32, b: i32, c: i32): i32 {
    let d;
    if true {
        let a = (2 * a) as i64;
        a += 12345;
        a *= 123;
    }
    if false {
        let a = (3 * a) as f64;
        a = 123.456;
        a -= 12.34;
    }
    {
        let a = 8 * a / 2;
        d = a;
    }
    return a * b + c * d;
}

pub fn main(): bool {
    let a: i32 = 42;
    {
        let a = a;
        a = 123456;
    }
    let c = 12;
    let d = fma(a, a, c);
    return d != 3780;
}
