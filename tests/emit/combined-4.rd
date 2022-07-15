// test: Test compiling multiple files
// build: $BINARY ./tests/emit/combined-second.rd -o=%.bc $TEST_ARGS && $BINARY % %.bc -o=%.out $TEST_ARGS
// cleanup: rm -f %.out %.bc

extern fn foo(a: int, b: int): int;

pub fn main(): int {
    if foo(20, 13) != 302 {
        return 1;
    }
    return 0;
}

