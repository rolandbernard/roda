// test:

pub fn main(): int {
    if !(1 > 0) {
        return 1;
    }
    if 0 > 0 {
        return 2;
    }
    if !(12 > -3) {
        return 3;
    }
    if -42 > -4 {
        return 4;
    }
    return 0;
}
