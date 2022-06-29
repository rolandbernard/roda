// test: Test extern global variables
// stderr: = Hello world! 42\n

type File = ();

extern let stderr: *File;
extern fn fprintf(file: *File, fmt: *u8, ..): i32;

pub fn main(): int {
    fprintf(stderr, "Hello world! %i\n", 42 as i32);
    return 0;
}

