// test: Compiler should be able to infer all types

fn test(a: int): int {
    return a;
}

pub fn main(a: int): int {
    let c;
    c = test(a);
    return 0;
}
