// test: Test moving around structs

fn foo(): uint {
    let a = (a = 5, b = [] as [0]int);
    return a.a;
}

pub fn main(): int {
    if (foo() != 5) {
        return 1;
    }
    return 0;
}

