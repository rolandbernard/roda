// test: Test moving around structs

fn foo(): i32 {
    let a;
    let b = (1, &a);
    a = b;
    return (*b.1).0;
}

pub fn main(): int {
    foo(); // Return value of this is undefined
    return 0;
}

