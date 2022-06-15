// test:

pub fn main(): int {
    if 0 >> 1 != 0 {
        return 1;
    }
    if 267240 >> 12 != 65 {
        return 2;
    }
    if 340 >> 3 != 42 {
        return 3;
    }
    if -35901923 >> 12 != -8766 {
        return 4;
    }
    return 0;
}
