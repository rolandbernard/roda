// test: test should be printed
// stdout: = test

extern fn printf(fmt: &u8);

pub fn main(): int {
    printf("test");
    return 0;
}
