// test:

pub fn main(): int {
    if 0 & 1 != 0 {
        return 1;
    }
    if 96 & 69 != 64 {
        return 2;
    }
    if 42 & 3 != 2 {
        return 3;
    }
    if -8765 & 12 != 0 {
        return 4;
    }
    return 0;
}
