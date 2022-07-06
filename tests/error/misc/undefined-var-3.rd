// test: Variables must be defined before use
// stderr: = error[E0003]: 11:13-15: use of undefined variable `foo`\n

fn bar() {
    fn foo(): int {
        return 0;
    }
}

pub fn main(): int {
    let a = foo();
    return 0;
}
