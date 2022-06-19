// test: Test moving around structs

fn foo(): uint {
    let b;
    let a = (a = b, b = [] as [0]int);
    return a.a;
}

pub fn main(): int {
    foo(); // Return value of this is undefined
    return 0;
}

