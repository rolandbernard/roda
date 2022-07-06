// test: Variables must be defined before use
// stderr: = error[E0003]: 9:13-15: use of undefined variable `foo`\n

fn bar() {
    const foo: int = 0;
}

pub fn main(): int {
    let a = foo;
    return 0;
}
