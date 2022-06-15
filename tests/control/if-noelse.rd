// test: Run if block only if condition is true

pub fn main(): int {
    if false {
        return 1;
    }
    if true {
        return 0;
    }
    return 2;
}
