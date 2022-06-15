// test:

pub fn main(): int {
    if !(0 <= 0) {
        return 1;
    }
    if 10 <= 0 {
        return 2;
    }
    if !(-3 <= 12) {
        return 3;
    }
    if -4 <= -42 {
        return 4;
    }
    return 0;
}
