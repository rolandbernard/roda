// test: Compiler should find a type conflict
// stderr: = error[E0008]: 5:34-37: type error, conflicting types `bool` and `int`\n

pub fn foo(): int {
    let a: (bool, int) = (false, true);
    return 0;
}
