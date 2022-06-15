// test: Compiler is unable to infer variable type
// stderr: = error[E0008]: 9:5-10: the left side of an assignment is not writable\n

fn test(): int {
    return 0;
}

pub fn foo(a: int): int {
    test() = a;
    return 0;
}
