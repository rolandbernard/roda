// test: Compiler should be able to infer all types

pub fn foo() {
    let a: [5]int;
    let b;
    let c = a[b];
}

pub fn main(): int {
    // We can not call foo, it is not safe
    return 0;
}
