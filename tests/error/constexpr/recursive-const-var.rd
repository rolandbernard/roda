// test: Test stuff that is not allowed in a constant expression
// stderr: = error[E0023]: 5:16-20: invalid value for constant definition\nerror[E0023]: 5:16-20: invalid value for constant definition\n

const x: int = y - 10;
const y: int = 2 * x;

pub fn main(): int {
    return 0;
}
