// test:

pub fn main(): int {
    if 0 * 0 != 0 {
        return 1;
    }
    if -12 * 12 != -144 {
        return 2;
    }
    if 42 * 100 != 4200 {
        return 3;
    }
    if 8765 * 1234 != 10816010 {
        return 4;
    }
    return 0;
}
