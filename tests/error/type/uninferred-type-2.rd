// test: Compiler is unable to infer variable type
// stderr: = error[E0011]: 7:9: type error, unable to infer the type of variable `c`\n

extern fn test(a: int, ..): int;

pub fn foo(a: int): int {
    let c;
    return test(a, c);
}
