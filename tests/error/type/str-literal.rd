// test: Compiler is unable to infer variable type
// stderr: = error[E0011]: 5:12-17: type error, expecting expression of type `int` but found string literal\n

pub fn foo(a: int): int {
    return "test";
}
