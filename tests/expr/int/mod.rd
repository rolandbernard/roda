// test:

pub fn main(): int {
    if 0 % 1 != 0 {
        return 1;
    }
    if 65 % 12 != 5 {
        return 2;
    }
    if 42 % 100 != 42 {
        return 3;
    }
    if -8765 % 1234 != -127 {
        return 4;
    }
    return 0;
}
