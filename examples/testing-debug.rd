
fn foo(test: bool): int {
    let a = 5;
    let c;
    let b = c;
    if (a == b) {
        return 12 + a;
    } else {
        a += b;
        return 5;
    }
    return 5;
}

