// test:

pub fn main(): int {
    if 0.0 != 0.0 {
        return 1;
    }
    if !(1.0 != 0.0) {
        return 2;
    }
    if !(3.0 != -1.2) {
        return 3;
    }
    if -4.2 != -4.2 {
        return 4;
    }
    return 0;
}
