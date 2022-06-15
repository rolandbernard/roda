// test: Compiler should find a type conflict
// stderr: = error[E0008]: 10:28: type error, conflicting types `bool` and `int`\n

pub fn foo(a: int): int {
    let b;
    let c;
    let d = c;
    let e;
    let f;
    if a == b && d == e && c {
        f = e;
        a += f;
    }
    return 0;
}
