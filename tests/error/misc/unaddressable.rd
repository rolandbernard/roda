// test: Compiler is unable to infer variable type
// stderr: = error[E0008]: 9:13-18: attempting to take pointer to expression that is not addressable\n

fn test(): int {
    return 0;
}

pub fn foo(): *int {
    return *test();
}
