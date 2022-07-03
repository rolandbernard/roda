// test: Test global variables

static a: int;
static b: int;
static c: int;

pub fn main(): int {
    a = 5;
    b = 2 * a;
    c = a + b;
    if c != 15 {
        return 1;
    }
    return 0;
}

