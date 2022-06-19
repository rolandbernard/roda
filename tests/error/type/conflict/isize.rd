// test: Compiler should find a type conflict
// stderr: = error[E0008]: 5:12: type error, conflicting types `int` and `isize`\n

pub fn foo(a: int): isize {
    return a;
}
