// test: Test stuff that is not allowed in a constant expression
// stderr: = error[E0009]: 4:17-26: sizeof expression not allowed in constant expressions\n

static b: int = sizeof int;

pub fn main(): int {
    return 0;
}
