// test: Compiler should give type error
// stderr: = error[E0012]: 5:9: type error, unsized type `[2]fn (): ()` for variable `a`\n

pub fn foo(): int {
    let a: [2]fn ();
    return 0;
}
