// test: Test stuff that is not allowed in a constant expression
// stderr: = error[E0008]: 5:13: type error, incompatible type `bool` for add expession, must be a numeric value\n

pub fn main(): int {
    let b: [1 + true]u8;
    return 0;
}
