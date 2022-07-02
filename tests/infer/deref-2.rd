// test: Compiler should be able to infer all types

pub fn foo(a: int) {
    let b;
    a = *b;
}

pub fn main(): int {
    // don't call foo. it is very unsafe
    return 0;
}
