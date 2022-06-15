// test:

pub fn main(): int {
    if 0 + 0 != 0 {
        return 1;
    }
    if -12 + 12 != 0 {
        return 2;
    }
    if 42 + 100 != 142 {
        return 3;
    }
    if 1234 + 8765 != 9999 {
        return 4;
    }
    return 0;
}
