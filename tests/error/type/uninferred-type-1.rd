// test: Compiler is unable to infer variable type
// stderr: = error[E0011]: 5:9: type error, unable to infer the type of variable `c`\n

pub fn foo(a: int): int {
    let c;
    return a;
}
