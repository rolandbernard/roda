// test: Test using usize

pub fn main(): int {
    let a: usize = 5;
    let b: usize = 7;
    let c = a + b;
    if c != 12 {
        return 1;
    }
    if sizeof usize != sizeof *() {
        return 2;
    }
    return 0;
}
