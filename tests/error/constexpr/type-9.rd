// test: Test stuff that is not allowed in a constant expression
// stderr: = error[E0008]: 5:27-29: type error, expecting expression of type `int` but found real literal\n

pub fn main(): int {
    const c: [2]int = [5, 7.5];
    return 0;
}
