// test: Compiler should find a type conflict
// stderr: = error[E0008]: 5:12-14: type error, expecting expression of type `int` but found real literal\n

pub fn foo(a: int): int {
    return 5.5;
}
