// test: Compiler should find a type conflict
// stderr: = error[E0008]: 5:25-28: type error, conflicting types `(int, f64)` and `(int)`\n

pub fn foo(): int {
    let a: (int, f64) = (5,);
    return 0;
}
