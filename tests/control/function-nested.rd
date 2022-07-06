// test: Should compile empty function

fn foo(): int {
    extern fn bar(a: int): int;
    fn fizz(a: int): int {
        return 2 * a;
    }
    return fizz(2) * 2;
}

pub fn main(): int {
    if foo() != 8 {
        return 1;
    }
    return 0;
}

