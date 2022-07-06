// test: Test global variables

fn foo(): int {
    static a: int = 5;
    static b: int = 2;
    static c: int = 0;

    b *= a;
    c += a + b;
    return c;
}

pub fn main(): int {
    if foo() != 15 {
        return 1;
    }
    if foo() != 70 {
        return 2;
    }
    return 0;
}

