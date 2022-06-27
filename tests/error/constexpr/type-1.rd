// test: Test stuff that is not allowed in a constant expression
// stderr: = error[E0008]: 5:17-19: type error, expecting expression of type `{integer}` but found real literal\n

pub fn main(): int {
    let b: [1 + 0.5]u8;
    return 0;
}
