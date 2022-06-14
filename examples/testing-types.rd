
fn bar(): [2]int {
    let a;
    return a;
}

fn foo(i: int): int {
    let a: [5]int;
    a[i] = 5;
    return bar()[i];
}

