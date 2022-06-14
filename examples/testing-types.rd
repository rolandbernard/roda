
import fn printf(fmt: *u8, ..);

fn foo(i: int): int {
    printf("Hello world! %li\n", i);
    return 0;
}

export fn main() {
    foo(42);
}

