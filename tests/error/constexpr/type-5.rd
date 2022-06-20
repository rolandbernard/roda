// test: Test stuff that is not allowed in a constant expression
// stderr: = error[E0008]: 5:19-23: type error, expecting expression of type `f64` but found boolean literal\n

pub fn main(): int {
    let b: [0.5 + false]u8;
    return 0;
}
