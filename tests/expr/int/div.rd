// test:

pub fn main(): int {
    if 0 / 1 != 0 {
        return 1;
    }
    if -12 / 12 != -1 {
        return 2;
    }
    if 42 / 100 != 0 {
        return 3;
    }
    if 8765 / 1234 != 7 {
        return 4;
    }
    return 0;
}
