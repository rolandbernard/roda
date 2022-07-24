// test: Test stuff that is not allowed in a constant expression
// stderr: = error[E0008]: 5:13-14: array length with non integer type `()`\n

pub fn main(): int {
    let b: [()]int;
    return 0;
}
