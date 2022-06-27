// test: Test stuff that is not allowed in a constant expression
// stderr: = error[E0008]: 5:13-17: type error, incompatible type `bool` for add expession, must be a numeric value\n

pub fn main(): int {
    let b: [false + 0.5]u8;
    return 0;
}
