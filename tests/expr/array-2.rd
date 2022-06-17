// test: Test loads and stores to arrays

fn foo(): [2]int {
    return [1, 2];
}

pub fn main(): int {
    let a = 1;
    if foo()[0] != 1 || foo()[1] != 2 {
        return 1;
    }
    if foo()[a] != 2 {
        return 2;
    }
    return 0;
}
