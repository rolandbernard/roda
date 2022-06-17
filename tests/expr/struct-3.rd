// test: Test moving around structs

pub fn main(): int {
    let a = 5;
    let b = (a = a, b = 42);
    if b.a != 5 || b.b != 42 {
        return 1;
    }
    return 0;
}
