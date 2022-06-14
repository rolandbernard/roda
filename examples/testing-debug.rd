
extern fn printf(fmt: *u8, ..);

fn foo(i: int): int {
    printf("Hello world! %li\n", i);
    return 0;
}

pub fn main(): int {
    foo(42);
    return 0;
}

