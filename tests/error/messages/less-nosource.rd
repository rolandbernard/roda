// test: Test style of error message
// build: ./build/$BUILD/bin/rodac $(realpath --relative-to=. %) --messages=less-nosource 2> %.out || true

extern fn test(a: f64): int;

pub fn foo(a: int): int {
    return test(a);
}
