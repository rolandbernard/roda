// test: Compiler should find a type conflict
// stderr: = error[E0008]: 5:35-37: type error, expecting expression of type `int` but found real literal\n

pub fn foo(a: int): int {
    return if a == 0 { 5 } else { 5.5 };
}
