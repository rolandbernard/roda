// test: Test stuff that is not allowed in a constant expression
// stderr: = error[E0023]: 5:26: in constant expression, recursive constant reference to `x`\n

const x: [5]int = [1, 2, y[1], 4, 5];
const y: [5]int = [5, 4, x[3], 3, 1];

pub fn main(): int {
    return 0;
}
