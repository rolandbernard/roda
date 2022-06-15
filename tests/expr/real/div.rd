// test:

pub fn main(): int {
    if 0.0 / 1.0 != 0.0 {
        return 1;
    }
    if -1.2 / 1.2 != -1.0 {
        return 2;
    }
    if 4.25 / 10.0 != 0.425 {
        return 3;
    }
    if 87.65 / 16.0 != 5.478125 {
        return 4;
    }
    return 0;
}
