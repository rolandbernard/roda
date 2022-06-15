// test:

pub fn main(): int {
    if 0.0 + 0.0 != 0.0 {
        return 1;
    }
    if -1.2 + 1.2 != 0.0 {
        return 2;
    }
    if 4.2 + 10.0 != 14.2 {
        return 3;
    }
    if 12.35 + 87.65 != 100.0 {
        return 4;
    }
    return 0;
}
