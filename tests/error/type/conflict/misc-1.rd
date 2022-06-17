// test: Compiler should find a type conflict
// stderr: = error[E0008]: 6:13-16: type error, expecting expression of type `int` but found boolean literal\n

pub fn foo(a: int): int {
    let b;
    let c = true;
    let d;
    let e;
    let f;
    if a == b && d == e && c == f {
        f = e;
        a += f;
    }
    return 0;
}
