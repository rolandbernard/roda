// test: Compiler should be able to infer all types

pub fn main(a: [5]int): int {
    let b;
    let c = a[b];
    return 0;
}
