// test: Nested functions, constants and globals can be shadowed by local variables

static a: int = 1;
fn foo(): int {
    const B: int = 20;
    fn fizz(): int {
        return a + b + A + B;
    }
    let a = 4;
    let A = 3;
    let b = 2;
    let B = 1;
    let fizz = a + b - A + B;
    return fizz;
    static b: int = 2;
}
const A: int = 10;

pub fn main(): int {
    if foo() != 4 {
        return 1;
    }
    return 0;
}

