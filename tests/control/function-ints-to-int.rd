// test: Should compile simple int -> int function

extern fn bar(a: int, b: int): int;

fn foo(a: int, b: int, c: int): int {
    return a + b + c;
}

pub fn fizz(a: int): int {
    return 2*a;
}

pub fn main(): bool {
    return fizz(foo(1, 2, 3)) != 12;
}

