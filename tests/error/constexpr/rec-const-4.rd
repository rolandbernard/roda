// test: Test stuff that is not allowed in a constant expression
// stderr: = error[E0023]: 5:20: in constant expression, recursive constant reference to `x`\n

const x: int = {
    const y: int = x;
    const z: int = 2 * y;
    3 * z
};

pub fn main(): int {
    return 0;
}
