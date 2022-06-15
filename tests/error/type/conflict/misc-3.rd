// test: Compiler should find a type conflict
// stderr: = error[E0008]: 13:12-20: type error, conflicting types `int` and `f64`\n

pub fn foo(a: int): f64 {
    let b;
    let c;
    let d = c;
    let e;
    let f;
    f = e;
    a += f;
    let g = b == c;
    return d + b - f;
}
