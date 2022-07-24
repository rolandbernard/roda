// test: Test stuff that is not allowed in a constant expression
// stderr: = error[E0008]: 5:23-31: type error, conflicting types `[2]int` and `[3]int`\n

pub fn main(): int {
    const c: [2]int = [5, 7, 8];
    return 0;
}
