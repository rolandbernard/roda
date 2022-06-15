// test: Compiler should find a type conflict
// stderr: = error[E0008]: 7:17: type error, conflicting types `int` and `f64`\n

extern fn test(a: f64): int;

pub fn foo(a: int): int {
    return test(a);
}
