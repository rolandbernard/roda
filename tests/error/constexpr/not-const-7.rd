// test: Test stuff that is not allowed in a constant expression
// stderr: = error[E0009]: 5:13-22: sizeof expression not allowed in constant expressions\n

pub fn main(): int {
    let b: [sizeof int]u8;
    return 0;
}
