// test:

pub fn main(): int {
    if 0 - 0 != 0 {
        return 1;
    }
    if -12 - 12 != -24 {
        return 2;
    }
    if 42 - 100 != -58 {
        return 3;
    }
    if 8765 - 1234 != 7531 {
        return 4;
    }
    return 0;
}
