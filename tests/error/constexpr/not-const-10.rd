// test: Test stuff that is not allowed in a constant expression
// stderr: = error[E0009]: 6:13-14: address of expression not allowed in constant expressions\n

pub fn main(): int {
    let a: int;
    let b: [*a]u8;
    return 0;
}
