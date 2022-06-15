// test: Compiler should be able to infer all types

pub fn main(a: int): int {
    let b;
    a = &b;
    return 0;
}
