// test: Should allow function that return integers

fn foo(): int {
    return 42;
}

pub fn main(): bool {
    return foo() != 42;
}
