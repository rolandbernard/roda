// test: Test moving around bool type

fn foo(a: bool, b: bool, c: bool): bool {
    return a && c || b;
}

pub fn main(): bool {
    let a: bool = true;
    let b = a;
    let c = false;
    let d = foo(a, b, c);
    return !d;
}
