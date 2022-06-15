// test: Compiler should find a type conflict
// stderr: = error[E0008]: 5:11: type error, conflicting types `bool` and `int`\n

pub fn foo(a: int): int {
    while a {
        a += 1;
    }
    return 0;
}
