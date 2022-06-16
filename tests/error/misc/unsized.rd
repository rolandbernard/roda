// test: Compiler should give type error
// stderr: = error[E0012]: 9:9: type error, unsized type `fn (): int` for variable `a`\n

fn test(): int {
    return 0;
}

pub fn foo(): int {
    let a = test;
    return 0;
}
