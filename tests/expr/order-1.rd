// test: Test the order of operations

pub fn main(): int {
    if 2 * 4 / 5 != 1 {
        return 1;
    }
    if 2 + 4 * 5 != 22 {
        return 2;
    }
    if (2 + 4) * 5 != 30 {
        return 3;
    }
    if 2 + 4 / 5 != 2 {
        return 4;
    }
    if 2 + -4 + 5 != 3 {
        return 5;
    }
    return 0;
}
