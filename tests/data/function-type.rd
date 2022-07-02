// test: Test moving around functions

fn foo(a: int, b: int): int {
    return 2 * a * b;
}

pub fn main(): bool {
    let a = &foo;
    let b = &a;
    let c = &b;
    return (***c)(2, 3) != 12;
}
