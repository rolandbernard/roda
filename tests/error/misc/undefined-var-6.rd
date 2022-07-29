// test: Nested functions can not access outer function arguments
// stderr: = error[E0002]: 7:16: can not capture variable `a` from outer function\n

pub fn foo(a: int): int {
    return bar();
    fn bar(): int {
        return a;
    }
}
