// test: Variables must be defined before use
// stderr: = error[E0003]: 5:12-16: use of undefined type `Undef`\n

pub fn main(): int {
    let a: Undef;
    return 0;
}
