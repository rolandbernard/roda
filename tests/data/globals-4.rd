// test: Test global variables

pub fn main(): int {
    b *= a;
    c += a + b;
    if c != 22 {
        return 1;
    }
    return 0;
}

static a: int = 5;
static b: int = 2;
static c: int = 7;

