// test: Test stuff that is not allowed in a constant expression
// stderr: = error[E0023]: 5:32: in constant expression, recursive constant reference to `x`\n

const x: (int, int, int) = (1, y.0, 5);
const y: (int, int, int) = (5, x.2, 1);

pub fn main(): int {
    return 0;
}
