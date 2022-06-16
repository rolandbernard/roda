// test: Test loads and stores to arrays

pub fn main(): int {
    let a: [3]int = [1, 2, 3];
    if a[0] != 1 || a[1] != 2 || a[2] != 3 {
        return 1;
    }
    a[0] += a[1] + a[2];
    if a[0] != 6 || a[1] != 2 || a[2] != 3 {
        return 2;
    }
    return 0;
}
