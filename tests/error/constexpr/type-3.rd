// test: Test stuff that is not allowed in a constant expression
// stderr: = error[E0008]: 5:13-16: array length with non integer type `bool`\n

pub fn main(): int {
    let b: [true]u8;
    return 0;
}
