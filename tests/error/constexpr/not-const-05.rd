// test: Test stuff that is not allowed in a constant expression
// stderr: = error[E0009]: 5:13-21: array literal expression not allowed in constant expressions\n

pub fn main(): int {
    let b: [[1, 2, 3]]int;
    return 0;
}
