// test: Test compiling multiple files
// build: ./build/$BUILD/bin/rodac ./tests/emit/combined-second.rd -o=%.o $TEST_ARGS && ./build/$BUILD/bin/rodac % %.o -o=%.out $TEST_ARGS
// cleanup: rm -f %.out %.o

extern fn foo(a: int, b: int): int;

pub fn main(): int {
    if foo(20, 13) != 302 {
        return 1;
    }
    return 0;
}

