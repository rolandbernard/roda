// test: Compiler should find a type conflict
// stderr: = error[E0008]: 5:12-26: type error, conflicting types `int` and `f64`\n

pub fn foo(a: int): f64 {
    return { 5.5; a + 10 };
}
