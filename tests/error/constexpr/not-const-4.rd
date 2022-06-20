// test: Test stuff that is not allowed in a constant expression
// stderr: = error[E0009]: 5:13-14: void expression not allowed in constant expressions\n

pub fn main(): int {
    let b: [()]int;
    return 0;
}
