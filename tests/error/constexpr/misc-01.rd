// test: Test stuff that is not allowed in a constant expression
// stderr: = error[E0023]: 6:22: in constant expession, array index `2` out of bounds for type `[2]int`\n

pub fn main(): int {
    const a: [2]int = [1, 2];
    const b: int = a[2];
    return 0;
}
