// test: Test some expressions with tuples

pub fn main(): int {
    let a: (i8, f64,) = (5, 5.5,);
    if a.0 != 5 || a.1 != 5.5 {
        return 1;
    }
    let b: (i8,) = (5,);
    if b.0 != 5 {
        return 2;
    }
    return 0;
}
