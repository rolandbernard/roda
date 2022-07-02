// test: Compiler should find a type conflict
// stderr: = error[E0008]: 5:25-35: type error, conflicting types `(int, f64)` and `(int, f64, {integer})`\n

pub fn foo(): int {
    let a: (int, f64) = (5, 5.5, 7);
    return 0;
}
