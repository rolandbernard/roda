// test: Recursive factorial implementation

fn fac(n: uint): uint {
    if n == 0 {
        return 1;
    } else {
        return n * fac(n - 1);
    }
}

pub fn main(): int {
    if fac(6) != 720 {
        return 1;
    }
    if fac(10) != 3628800 {
        return 2;
    }
    if fac(14) != 87178291200 {
        return 3;
    }
    return 0;
}
