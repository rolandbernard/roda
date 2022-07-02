// test: Test some expressions with tuples

pub fn main(): int {
    let a: (i8, f64) = (5, 5.5);
    let b = a;
    let c: (i8) = (a.0,);
    let d = c.0;
    if d != 5 {
        return 1;
    }
    let e = b.1;
    if e != 5.5 {
        return 2;
    }
    return 0;
}
