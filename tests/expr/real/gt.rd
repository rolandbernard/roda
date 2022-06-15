// test:

pub fn main(): int {
    if !(0.1 > 0.0) {
        return 1;
    }
    if 0.0 > 0.0 {
        return 2;
    }
    if !(1.2 > -0.3) {
        return 3;
    }
    if -0.42 > -0.4 {
        return 4;
    }
    return 0;
}
