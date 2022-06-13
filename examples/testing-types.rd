
fn add(a: int, b: int): int {
    return a + b;
}

export fn foo(test: bool): int {
    let a = true;
    let b = false;
    let c = a;
    if (test) {
        return add(5, 6);
    } else {
        return 0;
    }
}

