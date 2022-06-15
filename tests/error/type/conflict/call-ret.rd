// test: Compiler should find a type conflict
// stderr: = error[E0008]: 8:12: type error, conflicting types `f64` and `int`\n

extern fn test(a: int): f64;

pub fn foo(a: int): int {
    let c = test(a);
    return c;
}
