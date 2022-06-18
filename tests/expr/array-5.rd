// test: Test loads and stores to arrays

pub fn main(): int {
    let a = [1, 2, 3, 4];
    let b = [a[0], a[1], a[2], a[3]];
    if a[0] != 1 || a[1] != 2 || a[2] != 3 || a[3] != 4 {
        return 0;
    }
    return 0;
}
