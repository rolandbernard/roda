// test: Test loads and stores to arrays (Slow when run with debug information)
// buildtime: < 10s

pub fn foo(): [512]int {
    let a;
    a[5] = 42;
    a[100] = 12;
    return a;
}

pub fn main(): int {
    let a = foo();
    return 0;
}
