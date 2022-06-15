// test: Test the order of operations

pub fn main(): int {
    if 4 | 5 * 2 != 14 {
        return 1;
    }
    if 2 | 4 & 5 != 6 {
        return 2;
    }
    if 4 | 4 ^ 5 != 5 {
        return 3;
    }
    if 4 | 4 << 5 != 132 {
        return 4;
    }
    if 4 + 4 << 5 != 256 {
        return 4;
    }
    return 0;
}
