// test: Test stuff that is not allowed in a constant expression
// stderr: = error[E0008]: 5:13-26: array length with non integer type `(b: {integer}, a: {integer})`\n

pub fn main(): int {
    let b: [(a = 5, b = 7)]int;
    return 0;
}
