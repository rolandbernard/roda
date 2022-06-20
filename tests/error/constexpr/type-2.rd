// test: Test stuff that is not allowed in a constant expression
// stderr: = error[E0008]: 5:17-20: type error, expecting expression of type `i64` but found boolean literal\n

pub fn main(): int {
    let b: [1 + true]u8;
    return 0;
}
