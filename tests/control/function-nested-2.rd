// test: Nested functions can access nested statics and constants

static a: int = 1;
fn foo(): int {
    const B: int = 20;
    fn fizz(): int {
        return a + b + A + B;
    }
    return fizz();
    static b: int = 2;
}
const A: int = 10;

pub fn main(): int {
    if foo() != 33 {
        return 1;
    }
    return 0;
}

