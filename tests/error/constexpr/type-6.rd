// test: Test stuff that is not allowed in a constant expression
// stderr: = error[E0008]: 5:38-40: type error, expecting expression of type `int` but found real literal\n

pub fn main(): int {
    const c: (a: int, b: int) = (a = 5.5, b = 7);
    return 0;
}
