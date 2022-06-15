// test: Variables must be defined before use
// stderr: = error[E0003]: 5:13: use of undefined variable `a`\n

pub fn main(): int {
    let a = a;
    return 0;
}
