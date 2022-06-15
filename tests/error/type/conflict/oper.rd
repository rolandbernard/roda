// test: Compiler should find a type conflict
// stderr: = error[E0008]: 5:16: type error, conflicting types `f64` and `int`\n

pub fn foo(a: int, b: f64): int {
    return a + b;
}
