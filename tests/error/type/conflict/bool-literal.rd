// test: Compiler should find a type conflict
// stderr: = error[E0008]: 5:12-15: type error, conflicting types `bool` and `int`\n

pub fn foo(a: int): int {
    return true;
}
