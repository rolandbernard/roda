// test: Test loads and stores to pointers

pub fn main(): int {
    let a = 5;
    let b = *a;
    let c = *a;
    if a != 5 || &b != 5 || &c != 5 {
        return 1;
    }
    &b = 42;
    if a != 42 || &b != 42 || &c != 42 {
        return 2;
    }
    &b *= 2 * &c;
    if a != 3528 || &b != 3528 || &c != 3528 {
        return 3;
    }
    if b != c || *a != b || *a != c {
        return 4;
    }
    return 0;
}
