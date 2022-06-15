// test: Hello world should be printed
// stdout: = Hello world! 42\n

extern fn printf(fmt: *u8, ..);

pub fn main(): int {
    printf("Hello world! %li\n", 42);
    return 0;
}

