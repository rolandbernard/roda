// test: Test stuff that is not allowed in a constant expression
// stderr: = error[E0009]: 6:13-15: struct index expression not allowed in constant expressions\n

pub fn main(): int {
    let a: (a: int) = (a = 6);
    let b: [a.a]u8;
    return 0;
}
