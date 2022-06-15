// test:

pub fn main(): int {
    if !(0.0 == 0.0) {
        return 1;
    }
    if 1.0 == 0.0 {
        return 2;
    }
    if 0.3 == -0.12 {
        return 3;
    }
    if !(-0.42 == -42e-2) {
        return 4;
    }
    return 0;
}
