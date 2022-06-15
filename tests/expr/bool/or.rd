// test:

pub fn main(): int {
    if false || false {
        return 1;
    }
    if !(false || true) {
        return 2;
    }
    if !(true || false) {
        return 3;
    }
    if !(true || true) {
        return 4;
    }
    return 0;
}
