// test: Test stuff that is not allowed in a constant expression
// stderr: = error[E0009]: 6:13: variable expression not allowed in constant expressions\n

pub fn main(): int {
    let a: int = 5;
    let b: [a]int;
    return 0;
}
