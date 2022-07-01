// test: Compiler should find a type conflict
// stderr: = error[E0008]: 5:31-46: type error, conflicting types `(a: int, b: f64)` and `(a: int, c: {real})`\n

pub fn foo() {
    let a: (a: int, b: f64) = (a = 0, c = 5.5);
}
