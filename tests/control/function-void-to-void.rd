// test: Should compile empty function

extern fn bar();

fn foo() { }

pub fn fizz() { }

pub fn main(): int {
    foo();
    fizz();
    return 0;
}

