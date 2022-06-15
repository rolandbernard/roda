// test: Variables must be defined before use
// stderr: = error[E0002]: 5:6-10: the type `TypeA` is already defined\n

type TypeA = int;
type TypeA = int;

pub fn main(): int {
    return 0;
}
