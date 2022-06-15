// test:

pub fn main(): int {
    if !(0.0 < 0.1) {
        return 1;
    }
    if 0.0 < 0.0 {
        return 2;
    }
    if !(-0.3 < 1.2) {
        return 3;
    }
    if -4.0 < -4.2 {
        return 4;
    }
    return 0;
}
