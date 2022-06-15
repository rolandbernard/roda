// test: Variables must be defined before use
// stderr: = error[E0003]: 5:13-17: use of undefined variable `undef`\n

pub fn main(): int {
    let a = undef;
    return 0;
}
