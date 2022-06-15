// test:

pub fn main(): int {
    if 0 << 1 != 0 {
        return 1;
    }
    if 65 << 12 != 266240 {
        return 2;
    }
    if 42 << 3 != 336 {
        return 3;
    }
    if -8765 << 12 != -35901440 {
        return 4;
    }
    return 0;
}
