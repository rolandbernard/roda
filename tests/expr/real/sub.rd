// test:

pub fn main(): int {
    if 0.0 - 0.0 != 0.0 {
        return 1;
    }
    if -1.2 - 1.2 != -2.4 {
        return 2;
    }
    if 4.25 - 0.25 != 4.0 {
        return 3;
    }
    if 8.765 - 1.235 != 7.53 {
        return 4;
    }
    return 0;
}
