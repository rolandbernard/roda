// test:

pub fn main(): int {
    if 0 ^ 1 != 1 {
        return 1;
    }
    if 96 ^ 69 != 37 {
        return 2;
    }
    if 42 ^ 3 != 41 {
        return 3;
    }
    if -8765 ^ 12 != -8753 {
        return 4;
    }
    return 0;
}
