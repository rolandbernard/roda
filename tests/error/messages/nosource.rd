// test: Test style of error message
// build: LLVM_PROFILE_FILE="profile/tests/%%p.profraw" ./build/$BUILD/bin/rodac $(realpath --relative-to=. %) --messages=nosource 2> %.out || true

extern fn test(a: f64): int;

pub fn foo(a: int): int {
    return test(a);
}
