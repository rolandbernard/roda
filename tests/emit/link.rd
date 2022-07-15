// test: Test compiling linking in library
// build: $BINARY % -lm -o=%.out $TEST_ARGS

extern fn sqrt(x: f64): f64;
extern fn log2(x: f64): f64;

pub fn main(): int {
    if sqrt(4.0) != 2.0 {
        return 1;
    }
    if log2(1024.0) != 10.0 {
        return 2;
    }
    return 0;
}

