// test: Test stuff that is not allowed in a constant expression
// stderr: = error[E0008]: 5:13-21: array length with non integer type `f64`\n

pub fn main(): int {
    let b: [0.5 - 1.0]u8;
    return 0;
}
