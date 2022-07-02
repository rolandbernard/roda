// test: Compiler should find a type conflict
// stderr: = error[E0008]: 5:12-17: type error, conflicting types `&u8` and `int`\n

pub fn foo(a: int): int {
    return "test";
}
