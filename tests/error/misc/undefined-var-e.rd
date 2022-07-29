// test: Nested functions can not access outer function variables
// stderr: = error[E0002]: 7:26: can not capture variable `a` from outer function\n

pub fn foo(): int {
    let a = 100;
    fn bar(): int {
        const foo: int = a;
        return foo;
    }
    return bar();
}
