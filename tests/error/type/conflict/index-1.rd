// test: Compiler should find a type conflict
// stderr: = error[E0011]: 5:13-15: type error, expecting expression of type `u64` but found real literal\n

pub fn foo(a: *int): int {
    let b = 0.5;
    return a[b];
}
