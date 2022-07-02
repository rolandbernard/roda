// test: Test using isize

pub fn main(): int {
    let a: isize = 5;
    let b: isize = -7;
    let c = a + b;
    if c != -2 {
        return 1;
    }
    if sizeof isize != sizeof &() {
        return 2;
    }
    return 0;
}
