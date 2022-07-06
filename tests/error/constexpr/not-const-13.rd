// test: Test stuff that is not allowed in a constant expression
// stderr: = error[E0009]: 4:16-25: sizeof expression not allowed in constant expressions\n

const b: int = sizeof int;

pub fn main(): int {
    return 0;
}
