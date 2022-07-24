// test: Test stuff that is not allowed in a constant expression
// stderr: = error[E0008]: 5:28-30: type error, expecting expression of type `int` but found real literal\n

pub fn main(): int {
    const c: (int, int) = (5.5, 7);
    return 0;
}
