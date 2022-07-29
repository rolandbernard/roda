// test: Nested functions can not access constants in different scope
// stderr: = error[E0003]: 9:16: use of undefined variable `a`\n

pub fn foo(b: int): int {
    if b == 0 {
        const a: int = 100;
    }
    fn bar(): int {
        return a;
    }
    return bar();
}

