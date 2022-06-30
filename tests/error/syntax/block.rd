// test: Compiler should find a type conflict
// stderr: = error[E0001]: 5:17: syntax error, unexpected `}`\n

pub fn foo(a: int): int {
    return { a; };
}
