// test: Test some expressions with tuples

pub fn main(): int {
    let a: (i8, f64);
    a.0 = 5;
    a.1 = 5.5;
    let b = a;
    let c: (i8, f64,) = b;
    let d = a.0;
    if d != 5 {
        return 1;
    }
    let e = a.1;
    if e != 5.5 {
        return 2;
    }
    return 0;
}
