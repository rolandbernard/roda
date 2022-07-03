// test: Test stuff that is not allowed in a constant expression
// stderr: = error[E0023]: 5:20: recursive reference in constant definition of `x`\n

const x: int = y - 10;
const y: int = 2 * x;

pub fn main(): int {
    return 0;
}
