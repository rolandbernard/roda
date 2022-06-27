// test: Compiler should find a type conflict
// stderr: = error[E0008]: 5:13-15: type error, expecting expression of type `{integer}` but found real literal\n

pub fn foo(a: [5]int): int {
    let b = 0.5;
    return a[b];
}
