// test: Test compiling multiple files
// build: LLVM_PROFILE_FILE="profile/tests/%%p.profraw" ./build/$BUILD/bin/rodac % ./tests/emit/combined-second.rd -o=%.out $TEST_ARGS

extern fn foo(a: int, b: int): int;

pub fn main(): int {
    if foo(20, 13) != 302 {
        return 1;
    }
    return 0;
}

