
export fn foo(test: bool): int {
    let a = true;
    let b = false;
    let c = a;
    if (test) {
        return 1;
    } else {
        return 0;
    }
}

