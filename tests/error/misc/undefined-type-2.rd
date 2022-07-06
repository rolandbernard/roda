// test: Variables must be defined before use
// stderr: = error[E0003]: 9:12-14: use of undefined type `Foo`\n

fn bar() {
    type Foo = int;
}

pub fn main(): int {
    let a: Foo = 0;
    return 0;
}
