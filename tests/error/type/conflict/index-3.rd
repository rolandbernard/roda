// test: Compiler should find a type conflict
// stderr: = error[E0008]: 6:14: type error, incompatible type `f64` for index expession, must be an integer value\n

pub fn foo(a: [5]int): int {
    let b: f64 = 0.5;
    return a[b];
}
