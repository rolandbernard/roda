// test: Variables must be defined before use
// stderr: = error[E0002]: 4:21: the variable `a` is already defined\n

pub fn main(a: int, a: int): int {
    return 0;
}
