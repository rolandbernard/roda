// test: Variables must be defined before use
// stderr: = error[E0002]: 6:9: the variable `a` is already defined\n

pub fn main(): int {
    let a = 0;
    let a = 2;
    return 0;
}
