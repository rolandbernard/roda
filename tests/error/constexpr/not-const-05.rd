// test: Test stuff that is not allowed in a constant expression
// stderr: = error[E0008]: 5:13-21: array length with non integer type `[3]{integer}`\n

pub fn main(): int {
    let b: [[1, 2, 3]]int;
    return 0;
}
