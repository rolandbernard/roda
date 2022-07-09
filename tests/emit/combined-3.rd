// test: Test compiling multiple files
// build: ./build/$BUILD/bin/rodac ./tests/emit/combined-second.rd -o=%.ll $TEST_ARGS && ./build/$BUILD/bin/rodac % %.ll -o=%.out $TEST_ARGS
// cleanup: rm -f %.out %.ll

extern fn foo(a: int, b: int): int;

pub fn main(): int {
    if foo(20, 13) != 302 {
        return 1;
    }
    return 0;
}

