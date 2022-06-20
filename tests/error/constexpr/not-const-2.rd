// test: Test stuff that is not allowed in a constant expression
// stderr: = error[E0009]: 6:13-16: index expression not allowed in constant expressions\n

pub fn main(): int {
    let a: [5]int = [1, 2, 3, 4, 5];
    let b: [a[1]]int;
    return 0;
}
