// test: Compiler should find a type conflict
// stderr: = error[E0008]: 5:8: type error, conflicting types `bool` and `int`\n

pub fn foo(a: int): int {
    if a {
        return 0;
    } else {
        return 0;
    }
}
