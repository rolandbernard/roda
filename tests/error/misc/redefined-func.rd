// test: Variables must be defined before use
// stderr: = error[E0002]: 6:8-11: the variable `main` is already defined\n

extern fn main(a: int);

pub fn main(): int {
    return 0;
}
