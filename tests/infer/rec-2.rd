// test: Compiler should be able to infer all types

pub fn main(): int {
    let b;
    b = *b;
    b = &b;
    b = & &b;
    return 0;
}

