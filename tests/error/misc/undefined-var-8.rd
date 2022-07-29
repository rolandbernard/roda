// test: Nested functions can not access outer function variables
// stderr: = error[E0002]: 8:16: can not capture variable `a` from outer function\n

pub fn foo(): int {
    let a = 100;
    return bar();
    fn bar(): int {
        return a;
    }
}
