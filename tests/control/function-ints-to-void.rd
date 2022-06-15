// test: Should compile simple int -> () function

extern fn bar(a: int, b: int);

fn foo(a: int, b: int, c: int) { }

pub fn fizz(a: int) { }

pub fn main(): int {
    foo(1, 2, 3);
    fizz(4);
    return 0;
}

