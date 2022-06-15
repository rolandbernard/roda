// test: Compiler should find a type conflict
// stderr: = error[E0008]: 5:13: type error, conflicting types `f64` and `int`\n

pub fn foo(a: int, b: f64): int {
    if a == b {
        return 0;
    } else {
        return 0;
    }
}
