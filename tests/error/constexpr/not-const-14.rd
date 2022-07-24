// test: Test stuff that is not allowed in a constant expression
// stderr: = error[E0009]: 5:17-26: sizeof expression not allowed in constant expressions\n

pub fn main(): int {
    let b: [[1, sizeof int, 3]]int;
    return 0;
}
