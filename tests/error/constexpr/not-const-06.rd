// test: Test stuff that is not allowed in a constant expression
// stderr: = error[E0009]: 5:13-26: struct literal expression not allowed in constant expressions\n

pub fn main(): int {
    let b: [(a = 5, b = 7)]int;
    return 0;
}
