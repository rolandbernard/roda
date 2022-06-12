
fn add(a: int, b: int, c: int): int {
    return a + b;
}

fn main(test: bool): int {
    let a: u32 = 5;
    let b;
    b = a;
    let c = *(5 + 6 + b);
}

fn foo(test: bool): int {
    let a = 5;
    let c;
    let b = c;
    if (a == b) {
        return 12 + a;
    } else {
        a += b;
    }
}

