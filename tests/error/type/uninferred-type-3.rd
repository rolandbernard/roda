// test: Compiler is unable to infer variable type
// stderr: = error[E0011]: 5:9: type error, unable to infer the type of variable `a`\nerror[E0011]: 6:9: type error, unable to infer the type of variable `b`\nerror[E0011]: 7:9: type error, unable to infer the type of variable `c`\n

pub fn foo(a: int): int {
    let a;
    let b;
    let c;
    return 0;
}
