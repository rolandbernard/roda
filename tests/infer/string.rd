// test: Compiler should be able to infer all types

pub fn main(): int {
    let b = 5;
    let c = &b;
    c = "Hello";
    return 0;
}
