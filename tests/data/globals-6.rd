// test: Test global variables

fn foo(): int {
    b *= a;
    c += a + b;
    return c;

    static a: int = 5;
    static b: int = 2;
    static c: int = 0;
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

