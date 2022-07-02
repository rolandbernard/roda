// test: Test moving around pointers

fn foo(a: &int, b: &&int): &int {
    *b = a;
    return a;
}

pub fn main(): bool {
    let x = 12;
    let a: int = 42;
    let b = &a;
    let c = &b;
    let d = foo(&x, c);
    return *d != 12;
}
