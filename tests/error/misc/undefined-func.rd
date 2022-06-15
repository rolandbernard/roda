// test: Variables must be defined before use
// stderr: = error[E0003]: 5:5-9: use of undefined variable `undef`\n

pub fn main(): int {
    undef();
    return 0;
}
