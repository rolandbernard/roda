// test: Test function pointers
// stdout: = test 42

fn add(a: int, b: int): int {
    return a + b;
}

fn mul(a: int, b: int): int {
    return a * b;
}

fn div(a: int, b: int): int {
    return a / b;
}

extern fn printf(fmt: *u8, ..);

pub fn main(): int {
    let a = *add;
    let b = *mul;
    let c = *div;
    let d = [a, b, c];
    let e: [3]*fn(int, int): int = d;
    if (&e[0])(3, 4) != 7 {
        return 1;
    }
    if (&e[1])(3, 4) != 12 {
        return 2;
    }
    if (&e[2])(33, 4) != 8 {
        return 3;
    }
    let f: *fn (*u8, ..) = *printf;
    (&f)("test %li", 42);
    return 0;
}
