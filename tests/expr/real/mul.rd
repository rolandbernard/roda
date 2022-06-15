// test:

pub fn main(): int {
    if 0.0 * 0.0 != 0.0 {
        return 1;
    }
    if -1.2 * 1.2 != -1.44 {
        return 2;
    }
    if 4.2 * 1e2 != 420.0 {
        return 3;
    }
    if 87.65 * 12.34 != 1081.6010 {
        return 4;
    }
    return 0;
}
